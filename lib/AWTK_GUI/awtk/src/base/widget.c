﻿/**
 * File:   widget.c
 * Author: AWTK Develop Team
 * Brief:  basic class of all widget
 *
 * Copyright (c) 2018 - 2022  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-01-13 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../tkc/mem.h"
#include "../tkc/utf8.h"
#include "../tkc/utils.h"
#include "../tkc/fscript.h"
#include "../tkc/tokenizer.h"
#include "../tkc/color_parser.h"
#include "../tkc/object_default.h"

#include "keys.h"
#include "enums.h"
#include "theme.h"
#include "../tkc/time_now.h"
#include "idle.h"
#include "widget.h"
#include "layout.h"
#include "native_window.h"
#include "main_loop.h"
#include "ui_feedback.h"
#include "system_info.h"
#include "window_manager.h"
#include "widget_vtable.h"
#include "style_mutable.h"
#include "style_factory.h"
#include "widget_animator_manager.h"
#include "widget_animator_factory.h"
#include "window_base.h"
#include "assets_manager.h"
#include "../blend/image_g2d.h"

ret_t widget_focus_up(widget_t *widget);
ret_t widget_focus_down(widget_t *widget);
ret_t widget_focus_left(widget_t *widget);
ret_t widget_focus_right(widget_t *widget);
static ret_t widget_unref_async(widget_t *widget);
static ret_t widget_ensure_style_mutable(widget_t *widget);
static ret_t widget_dispatch_blur_event(widget_t *widget);
/*虚函数的包装*/
static ret_t widget_on_paint_done(widget_t *widget, canvas_t *c);
static ret_t widget_on_paint_begin(widget_t *widget, canvas_t *c);
static ret_t widget_on_paint_end(widget_t *widget, canvas_t *c);

typedef widget_t *(*widget_find_wanted_focus_widget_t)(widget_t *widget, darray_t *all_focusable);
static ret_t widget_move_focus(widget_t *widget, widget_find_wanted_focus_widget_t find);

#define widget_set_xywh(widget, val, update_layout, invalidate)    \
  do                                                               \
  {                                                                \
    if (widget->val != val)                                        \
    {                                                              \
      if (invalidate)                                              \
        widget_invalidate_force(widget, NULL);                     \
      widget->val = val;                                           \
      if (invalidate)                                              \
        widget_invalidate_force(widget, NULL);                     \
    }                                                              \
    if (update_layout && widget->self_layout != NULL)              \
    {                                                              \
      self_layouter_set_param_str(widget->self_layout, #val, "n"); \
    }                                                              \
  } while (0)

static ret_t widget_set_x(widget_t *widget, xy_t x, bool_t update_layout)
{
  widget_set_xywh(widget, x, update_layout, TRUE);
  return RET_OK;
}

static ret_t widget_set_y(widget_t *widget, xy_t y, bool_t update_layout)
{
  widget_set_xywh(widget, y, update_layout, TRUE);
  return RET_OK;
}

static ret_t widget_set_w(widget_t *widget, wh_t w, bool_t update_layout)
{
  widget_set_xywh(widget, w, update_layout, TRUE);
  return RET_OK;
}

static ret_t widget_set_h(widget_t *widget, xy_t h, bool_t update_layout)
{
  widget_set_xywh(widget, h, update_layout, TRUE);
  return RET_OK;
}

static bool_t widget_is_strongly_focus(widget_t *widget)
{
  widget_t *win = widget_get_window(widget);
  if (win != NULL)
  {
    return WINDOW_BASE(win)->strongly_focus;
  }
  else
  {
    return FALSE;
  }
}

ret_t widget_set_need_update_style(widget_t *widget)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (!widget->need_update_style)
  {
    widget_invalidate_force(widget, NULL);
  }

  widget->need_update_style = TRUE;

  return RET_OK;
}

ret_t widget_update_style_recursive(widget_t *widget)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget_update_style(widget);
  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  widget_update_style_recursive(iter);
  WIDGET_FOR_EACH_CHILD_END();

  return RET_OK;
}

ret_t widget_set_need_update_style_recursive(widget_t *widget)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget_set_need_update_style(widget);
  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  widget_set_need_update_style_recursive(iter);
  WIDGET_FOR_EACH_CHILD_END();

  return RET_OK;
}

ret_t widget_update_style(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->astyle != NULL, RET_BAD_PARAMS);

  if (widget->need_update_style)
  {
    widget->need_update_style = FALSE;
    return style_notify_widget_state_changed(widget->astyle, widget);
  }

  return RET_OK;
}

static ret_t widget_real_destroy(widget_t *widget)
{
  ENSURE(widget->ref_count == 1);

  if (widget->vt->on_destroy)
  {
    widget->vt->on_destroy(widget);
  }

  TKMEM_FREE(widget->name);
  TKMEM_FREE(widget->state);
  TKMEM_FREE(widget->style);
  TKMEM_FREE(widget->tr_text);
  TKMEM_FREE(widget->animation);
  TKMEM_FREE(widget->pointer_cursor);
  TK_OBJECT_UNREF(widget->custom_props);
  wstr_reset(&(widget->text));
  style_destroy(widget->astyle);

  memset(widget, 0x00, sizeof(widget_t));
  TKMEM_FREE(widget);

  return RET_OK;
}

static widget_t *widget_real_create(const widget_vtable_t *vt)
{
  widget_t *widget = TKMEM_ALLOC(vt->size);
  return_value_if_fail(widget != NULL, NULL);

  memset(widget, 0x00, vt->size);
  widget->vt = vt;

  return widget;
}

static bool_t widget_is_scrollable(widget_t *widget)
{
  return widget != NULL && widget->vt != NULL && widget->vt->scrollable;
}

static bool_t widget_with_focus_state(widget_t *widget)
{
  value_t v;
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);
  value_set_bool(&v, FALSE);
  widget_get_prop(widget, WIDGET_PROP_WITH_FOCUS_STATE, &v);

  return value_bool(&v);
}

bool_t widget_is_focusable(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  if (!widget->visible || !widget->sensitive || !widget->enable)
  {
    return FALSE;
  }

  return widget->focusable || widget->vt->focusable;
}

ret_t widget_move(widget_t *widget, xy_t x, xy_t y)
{
  event_t e = event_init(EVT_WILL_MOVE, widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->x != x || widget->y != y)
  {
    widget_dispatch(widget, &e);

    widget_invalidate_force(widget, NULL);
    widget_set_xywh(widget, x, TRUE, FALSE);
    widget_set_xywh(widget, y, TRUE, FALSE);
    widget_invalidate_force(widget, NULL);

    e.type = EVT_MOVE;
    widget_dispatch(widget, &e);
  }

  return RET_OK;
}

ret_t widget_move_to_center(widget_t *widget)
{
  int32_t x = 0;
  int32_t y = 0;
  return_value_if_fail(widget != NULL && widget->parent != NULL, RET_BAD_PARAMS);

  x = (widget->parent->w - widget->w) / 2;
  y = (widget->parent->h - widget->h) / 2;

  return widget_move(widget, x, y);
}

ret_t widget_resize(widget_t *widget, wh_t w, wh_t h)
{
  event_t e = event_init(EVT_WILL_RESIZE, widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->w != w || widget->h != h)
  {
    widget_dispatch(widget, &e);

    widget_invalidate_force(widget, NULL);
    widget_set_xywh(widget, w, TRUE, FALSE);
    widget_set_xywh(widget, h, TRUE, FALSE);
    widget_invalidate_force(widget, NULL);
    widget_set_need_relayout_children(widget);

    e.type = EVT_RESIZE;
    widget_dispatch(widget, &e);
  }

  return RET_OK;
}

ret_t widget_move_resize_ex(widget_t *widget, xy_t x, xy_t y, wh_t w, wh_t h,
                            bool_t update_layout)
{
  event_t e = event_init(EVT_WILL_MOVE_RESIZE, widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->x != x || widget->y != y || widget->w != w || widget->h != h)
  {
    widget_dispatch(widget, &e);

    widget_invalidate_force(widget, NULL);
    widget_set_xywh(widget, x, update_layout, FALSE);
    widget_set_xywh(widget, y, update_layout, FALSE);
    widget_set_xywh(widget, w, update_layout, FALSE);
    widget_set_xywh(widget, h, update_layout, FALSE);
    widget_invalidate_force(widget, NULL);
    widget_set_need_relayout_children(widget);

    e.type = EVT_MOVE_RESIZE;
    widget_dispatch(widget, &e);
  }

  return RET_OK;
}

ret_t widget_move_resize(widget_t *widget, xy_t x, xy_t y, wh_t w, wh_t h)
{
  return widget_move_resize_ex(widget, x, y, w, h, TRUE);
}

float_t widget_get_value(widget_t *widget)
{
  value_t v;
  return_value_if_fail(widget != NULL, 0);

  return widget_get_prop(widget, WIDGET_PROP_VALUE, &v) == RET_OK ? value_float32(&v) : 0.0f;
}

ret_t widget_set_value(widget_t *widget, float_t value)
{
  value_t v;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  return widget_set_prop(widget, WIDGET_PROP_VALUE, value_set_float32(&v, value));
}

ret_t widget_add_value(widget_t *widget, float_t delta)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  return widget_set_value(widget, widget_get_value(widget) + delta);
}

int32_t widget_get_value_int(widget_t *widget)
{
  value_t v;
  return_value_if_fail(widget != NULL, 0);

  return widget_get_prop(widget, WIDGET_PROP_VALUE, &v) == RET_OK ? value_int(&v) : 0;
}

ret_t widget_set_value_int(widget_t *widget, int32_t value)
{
  value_t v;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  return widget_set_prop(widget, WIDGET_PROP_VALUE, value_set_int(&v, value));
}

ret_t widget_add_value_int(widget_t *widget, int32_t delta)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  return widget_set_value_int(widget, widget_get_value_int(widget) + delta);
}

static ret_t widget_animate_prop_float_to(widget_t *widget, const char *name, float_t value,
                                          uint32_t duration)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && name != NULL, RET_BAD_PARAMS);
  if (duration == 0)
  {
    ret = widget_set_prop_float(widget, name, value);
  }
  else
  {
    float_t prev_value = widget_get_prop_float(widget, name, 0.0f);
    widget_destroy_animator(widget, name);

    if (prev_value != value)
    {
      char params[128] = {0};
      tk_snprintf(params, sizeof(params) - 1, "%s(from=%f,to=%f,duration=%d)", name, prev_value,
                  value, duration);
      ret = widget_create_animator(widget, params);
    }
  }
  return ret;
}

ret_t widget_animate_value_to(widget_t *widget, float_t value, uint32_t duration)
{
  return widget_animate_prop_float_to(widget, WIDGET_PROP_VALUE, value, duration);
}

bool_t widget_is_window_opened(widget_t *widget)
{
  widget_t *win = widget_get_window(widget);

  if (win != NULL)
  {
    int32_t stage = widget_get_prop_int(win, WIDGET_PROP_STAGE, WINDOW_STAGE_NONE);
    return WINDOW_STAGE_OPENED == stage || WINDOW_STAGE_SUSPEND == stage;
  }
  else
  {
    return FALSE;
  }
}

bool_t widget_is_window_created(widget_t *widget)
{
  widget_t *win = widget_get_window(widget);

  if (win != NULL)
  {
    int32_t stage = widget_get_prop_int(win, WIDGET_PROP_STAGE, WINDOW_STAGE_NONE);
    return WINDOW_STAGE_OPENED == stage || WINDOW_STAGE_SUSPEND == stage ||
           WINDOW_STAGE_LOADED == stage || WINDOW_STAGE_CREATED == stage;
  }
  else
  {
    return FALSE;
  }
}

ret_t widget_get_window_theme(widget_t *widget, theme_t **win_theme, theme_t **default_theme)
{
  value_t v;
  widget_t *win = widget_get_window(widget);

  if (win != NULL)
  {
    if (widget_get_prop(win, WIDGET_PROP_THEME_OBJ, &v) == RET_OK)
    {
      *win_theme = (theme_t *)value_pointer(&v);
    }

    if (widget_get_prop(win, WIDGET_PROP_DEFAULT_THEME_OBJ, &v) == RET_OK)
    {
      *default_theme = (theme_t *)value_pointer(&v);
    }
  }
  return RET_OK;
}

bool_t widget_is_style_exist(widget_t *widget, const char *style_name, const char *state_name)
{
  const void *data = NULL;
  const char *style = NULL;
  const char *state = NULL;
  theme_t *win_theme = NULL;
  theme_t *default_theme = NULL;
  const char *type = widget_get_type(widget);
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(widget != NULL && win != NULL, FALSE);

  if (style_name == NULL || *style_name == 0)
  {
    style = TK_DEFAULT_STYLE;
  }
  else
  {
    style = style_name;
  }

  if (state_name == NULL || *state_name == 0)
  {
    state = WIDGET_STATE_NORMAL;
  }
  else
  {
    state = state_name;
  }

  return_value_if_fail(widget_get_window_theme(widget, &win_theme, &default_theme) == RET_OK,
                       FALSE);

  if (win_theme != NULL)
  {
    data = theme_find_style(win_theme, type, style, state);
  }

  if (data == NULL && default_theme != NULL)
  {
    data = theme_find_style(default_theme, type, style, state);
  }

  return data != NULL;
}

ret_t widget_use_style(widget_t *widget, const char *value)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget_set_need_update_style(widget);
  widget->style = tk_str_copy(widget->style, value);

  if (widget_is_window_opened(widget))
  {
    widget_update_style(widget);
    return widget_invalidate(widget, NULL);
  }

  return RET_OK;
}

ret_t widget_set_text_impl(widget_t *widget, const wchar_t *text, bool_t check_diff)
{
  value_t v;
  if (check_diff && tk_wstr_eq(widget->text.str, text))
  {
    return RET_NOT_MODIFIED;
  }
  return widget_set_prop(widget, WIDGET_PROP_TEXT, value_set_wstr(&v, text));
}

ret_t widget_set_text_utf8_impl(widget_t *widget, const char *text, bool_t check_diff)
{
  value_t v;
  if (check_diff)
  {
    wstr_t str;
    ret_t ret = RET_NOT_MODIFIED;
    uint32_t len = tk_strlen(text);
    if (len != widget->text.size)
    {
      return widget_set_prop(widget, WIDGET_PROP_TEXT, value_set_str(&v, text));
    }
    return_value_if_fail(wstr_init(&str, len + 2) != NULL, RET_OOM);
    tk_utf8_to_utf16(text, str.str, str.capacity - 1);

    if (!tk_wstr_eq(widget->text.str, str.str))
    {
      widget_set_prop(widget, WIDGET_PROP_TEXT, value_set_wstr(&v, str.str));
      ret = RET_OK;
    }
    wstr_reset(&str);
    return ret;
  }
  else
  {
    return widget_set_prop(widget, WIDGET_PROP_TEXT, value_set_str(&v, text));
  }
}

ret_t widget_set_text_ex(widget_t *widget, const wchar_t *text, bool_t check_diff)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  ret = widget_set_text_impl(widget, text, check_diff);
  if (ret == RET_NOT_MODIFIED)
  {
    ret = RET_OK;
  }
  return ret;
}

ret_t widget_set_text_utf8_ex(widget_t *widget, const char *text, bool_t check_diff)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);
  ret = widget_set_text_utf8_impl(widget, text, check_diff);
  if (ret == RET_NOT_MODIFIED)
  {
    ret = RET_OK;
  }
  return ret;
}

ret_t widget_set_text(widget_t *widget, const wchar_t *text)
{
  return widget_set_text_ex(widget, text, TRUE);
}

ret_t widget_set_text_utf8(widget_t *widget, const char *text)
{
  return widget_set_text_utf8_ex(widget, text, TRUE);
}

ret_t widget_get_text_utf8(widget_t *widget, char *text, uint32_t size)
{
  value_t v;
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && text != NULL && size > 0, RET_BAD_PARAMS);

  value_set_str(&v, NULL);
  memset(text, 0x00, size);
  if (widget_get_prop(widget, WIDGET_PROP_TEXT, &v) == RET_OK)
  {
    if (v.type == VALUE_TYPE_STRING)
    {
      tk_strncpy(text, value_str(&v), size - 1);
      ret = RET_OK;
    }
    else if (v.type == VALUE_TYPE_WSTRING)
    {
      tk_utf8_from_utf16(value_wstr(&v), text, size);
      ret = RET_OK;
    }
  }

  return ret;
}

image_manager_t *widget_get_image_manager(widget_t *widget)
{
  image_manager_t *ret = image_manager();
  return_value_if_fail(widget != NULL && widget->vt != NULL, ret);

  if (tk_str_eq(widget->vt->type, WIDGET_TYPE_WINDOW_MANAGER))
  {
    ret = image_manager();
  }
  else
  {
    value_t v;
    widget_t *win = widget_get_window(widget);
    if (widget_get_prop(win, WIDGET_PROP_IMAGE_MANAGER, &v) == RET_OK)
    {
      ret = (image_manager_t *)value_pointer(&v);
    }
  }

  return ret;
}

locale_info_t *widget_get_locale_info(widget_t *widget)
{
  locale_info_t *ret = locale_info();
  return_value_if_fail(widget != NULL && widget->vt != NULL, ret);

  if (tk_str_eq(widget->vt->type, WIDGET_TYPE_WINDOW_MANAGER))
  {
    ret = locale_info();
  }
  else
  {
    value_t v;
    widget_t *win = widget_get_window(widget);
    if (widget_get_prop(win, WIDGET_PROP_LOCALE_INFO, &v) == RET_OK)
    {
      ret = (locale_info_t *)value_pointer(&v);
    }
  }

  return ret;
}

assets_manager_t *widget_get_assets_manager(widget_t *widget)
{
  assets_manager_t *am = assets_manager();
  return_value_if_fail(widget != NULL && widget->vt != NULL, am);

  if (widget->assets_manager != NULL)
  {
    return widget->assets_manager;
  }

  if (tk_str_eq(widget->vt->type, WIDGET_TYPE_WINDOW_MANAGER))
  {
    am = assets_manager();
  }
  else
  {
    value_t v;
    widget_t *win = widget_get_window(widget);
    if (widget_get_prop(win, WIDGET_PROP_ASSETS_MANAGER, &v) == RET_OK)
    {
      am = (assets_manager_t *)value_pointer(&v);
    }
  }
  widget->assets_manager = am;

  return am;
}

font_manager_t *widget_get_font_manager(widget_t *widget)
{
  font_manager_t *ret = font_manager();
  return_value_if_fail(widget != NULL && widget->vt != NULL, ret);

  if (tk_str_eq(widget->vt->type, WIDGET_TYPE_WINDOW_MANAGER))
  {
    ret = font_manager();
  }
  else
  {
    value_t v;
    widget_t *win = widget_get_window(widget);
    if (widget_get_prop(win, WIDGET_PROP_FONT_MANAGER, &v) == RET_OK)
    {
      ret = (font_manager_t *)value_pointer(&v);
    }
  }

  return ret;
}

static ret_t widget_apply_tr_text_before_paint(void *ctx, event_t *e)
{
  widget_t *widget = WIDGET(ctx);
  if (widget->tr_text != NULL)
  {
    const char *text = locale_info_tr(widget_get_locale_info(widget), widget->tr_text);
    widget_set_text_utf8_impl(widget, text, FALSE);
  }

  return RET_REMOVE;
}

ret_t widget_set_tr_text(widget_t *widget, const char *text)
{
  const char *tr_text = NULL;
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(widget != NULL, RET_OK);

  if (text == NULL || *text == '\0')
  {
    if (widget->tr_text != NULL)
    {
      TKMEM_FREE(widget->tr_text);
      widget_set_text_utf8_impl(widget, text, FALSE);
      widget_invalidate(widget, NULL);
    }

    return RET_OK;
  }

  widget->tr_text = tk_str_copy(widget->tr_text, text);
  if (win != NULL)
  {
    tr_text = locale_info_tr(widget_get_locale_info(widget), text);
    widget_set_text_utf8_impl(widget, tr_text, TRUE);
  }
  else
  {
    widget_set_text_utf8_impl(widget, text, FALSE);
    widget_on(widget, EVT_BEFORE_PAINT, widget_apply_tr_text_before_paint, widget);
  }

  return RET_OK;
}

ret_t widget_re_translate_text(widget_t *widget)
{
  if (widget->vt->on_re_translate != NULL)
  {
    widget->vt->on_re_translate(widget);
  }
  if (widget->tr_text != NULL)
  {
    const char *tr_text = locale_info_tr(widget_get_locale_info(widget), widget->tr_text);
    widget_set_text_utf8_impl(widget, tr_text, FALSE);
    widget_invalidate(widget, NULL);
  }

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  widget_re_translate_text(iter);
  WIDGET_FOR_EACH_CHILD_END();

  return RET_OK;
}

const wchar_t *widget_get_text(widget_t *widget)
{
  value_t v;
  return_value_if_fail(widget != NULL, 0);

  return widget_get_prop(widget, WIDGET_PROP_TEXT, &v) == RET_OK ? value_wstr(&v) : 0;
}

ret_t widget_set_name(widget_t *widget, const char *name)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (name != NULL)
  {
    widget->name = tk_str_copy(widget->name, name);
  }
  else
  {
    TKMEM_FREE(widget->name);
  }

  return RET_OK;
}

const char *widget_get_theme_name(widget_t *widget)
{
  return_value_if_fail(widget != NULL, NULL);
  assets_manager_t *am = widget_get_assets_manager(widget);
  return_value_if_fail(am != NULL, NULL);

  return assets_manager_get_theme_name(am);
}

ret_t widget_set_theme(widget_t *widget, const char *name)
{
  theme_change_event_t will_event;
  event_t *will_evt = theme_change_event_init(&will_event, EVT_THEME_WILL_CHANGE, name);
  widget_dispatch(window_manager(), will_evt);
#if defined(WITH_FS_RES) && !defined(AWTK_WEB)
  {
    const asset_info_t *info = NULL;
    theme_change_event_t event;
    event_t *evt = theme_change_event_init(&event, EVT_THEME_CHANGED, name);
    widget_t *wm = widget_get_window_manager(widget);
    assets_manager_t *am = widget_get_assets_manager(widget);
    locale_info_t *locale_info = widget_get_locale_info(widget);
    return_value_if_fail(am != NULL && name != NULL, RET_BAD_PARAMS);

    font_managers_unload_all();
    image_managers_unload_all();
    locale_infos_reload_all();
    assets_managers_set_theme(name);
    widget_reset_canvas(widget);

    info = assets_manager_ref(am, ASSET_TYPE_STYLE, "default");
    if (info != NULL)
    {
      theme_set(theme_load_from_data(info->name, info->data, info->size));
      assets_manager_unref(assets_manager(), info);
    }

    widget_dispatch(wm, evt);
    widget_invalidate_force(wm, NULL);

    log_debug("theme changed: %s\n", name);
  }
#endif /*defined(WITH_FS_RES) && !defined(AWTK_WEB)*/

  return RET_OK;
}

ret_t widget_set_pointer_cursor(widget_t *widget, const char *cursor)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (!tk_str_eq(widget->pointer_cursor, cursor))
  {
    widget->pointer_cursor = tk_str_copy(widget->pointer_cursor, cursor);
    widget_update_pointer_cursor(widget);
  }

  return RET_OK;
}

#ifndef WITHOUT_WIDGET_ANIMATORS
ret_t widget_set_animation(widget_t *widget, const char *animation)
{
  return_value_if_fail(widget != NULL && animation != NULL, RET_BAD_PARAMS);

  widget->animation = tk_str_copy(widget->animation, animation);

  return widget_create_animator(widget, animation);
}

ret_t widget_create_animator(widget_t *widget, const char *animation)
{
  tokenizer_t t;
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && animation != NULL, RET_BAD_PARAMS);
  return_value_if_fail(tokenizer_init(&t, animation, strlen(animation), ";") != NULL, RET_OOM);

  while (tokenizer_has_more(&t))
  {
    const char *params = tokenizer_next(&t);
    if (widget_animator_create(widget, params) == NULL)
    {
      ret = RET_BAD_PARAMS;
      break;
    }
  }
  tokenizer_deinit(&t);
  widget_invalidate(widget, NULL);

  return ret;
}

ret_t widget_start_animator(widget_t *widget, const char *name)
{
  return widget_animator_manager_start(widget_animator_manager(), widget, name);
}

ret_t widget_set_animator_time_scale(widget_t *widget, const char *name, float_t time_scale)
{
  return widget_animator_manager_set_time_scale(widget_animator_manager(), widget, name,
                                                time_scale);
}

ret_t widget_pause_animator(widget_t *widget, const char *name)
{
  return widget_animator_manager_pause(widget_animator_manager(), widget, name);
}

widget_animator_t *widget_find_animator(widget_t *widget, const char *name)
{
  return widget_animator_manager_find(widget_animator_manager(), widget, name);
}

ret_t widget_stop_animator(widget_t *widget, const char *name)
{
  return widget_animator_manager_stop(widget_animator_manager(), widget, name);
}

ret_t widget_destroy_animator(widget_t *widget, const char *name)
{
  return widget_animator_manager_remove_all(widget_animator_manager(), widget, name);
}
#else
ret_t widget_start_animator(widget_t *widget, const char *name)
{
  return RET_OK;
}
ret_t widget_create_animator(widget_t *widget, const char *animation)
{
  return RET_OK;
}
ret_t widget_set_animation(widget_t *widget, const char *animation)
{
  return RET_OK;
}
ret_t widget_set_animator_time_scale(widget_t *widget, const char *name, float_t time_scale)
{
  return RET_OK;
}
ret_t widget_pause_animator(widget_t *widget, const char *name)
{
  return RET_OK;
}
widget_animator_t *widget_find_animator(widget_t *widget, const char *name)
{
  return NULL;
}
ret_t widget_stop_animator(widget_t *widget, const char *name)
{
  return RET_OK;
}
ret_t widget_destroy_animator(widget_t *widget, const char *name)
{
  return RET_OK;
}
#endif /*WITHOUT_WIDGET_ANIMATORS*/

ret_t widget_set_enable(widget_t *widget, bool_t enable)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->enable != enable)
  {
    widget->enable = enable;
    widget_set_need_update_style_recursive(widget);
    widget_invalidate(widget, NULL);
  }

  return RET_OK;
}

ret_t widget_set_feedback(widget_t *widget, bool_t feedback)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->feedback = feedback;

  return RET_OK;
}

ret_t widget_set_auto_adjust_size(widget_t *widget, bool_t auto_adjust_size)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->auto_adjust_size = auto_adjust_size;
  widget_set_need_relayout(widget);

  return RET_OK;
}

ret_t widget_set_floating(widget_t *widget, bool_t floating)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->floating = floating;

  return RET_OK;
}

ret_t widget_set_focused_internal(widget_t *widget, bool_t focused)
{
  widget_t *win = widget_get_window(widget);
  int32_t stage = widget_get_prop_int(win, WIDGET_PROP_STAGE, WINDOW_STAGE_NONE);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (WINDOW_STAGE_SUSPEND == stage)
  {
    log_debug("You can not set focus of a widget when window is in background");
    return RET_FAIL;
  }

  if (widget->focused != focused)
  {
    widget->focused = focused;
    widget_set_need_update_style(widget);

    if (focused)
    {
      event_t e = event_init(EVT_FOCUS, widget);
      widget_set_as_key_target(widget);

      widget_dispatch(widget, &e);
    }
    else
    {
      event_t e = event_init(EVT_BLUR, widget);
      widget_dispatch(widget, &e);
      widget_dispatch_blur_event(widget);
    }

    widget_invalidate(widget, NULL);
  }

  return RET_OK;
}

ret_t widget_set_focused(widget_t *widget, bool_t focused)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget_set_focused_internal(widget, focused);
  if (focused)
  {
    widget_ensure_visible_in_viewport(widget);
  }

  return RET_OK;
}

ret_t widget_set_focusable(widget_t *widget, bool_t focusable)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->focusable = focusable;

  return RET_OK;
}

ret_t widget_set_state(widget_t *widget, const char *state)
{
  return_value_if_fail(widget != NULL && state != NULL, RET_BAD_PARAMS);

  if (!tk_str_eq(widget->state, state))
  {
    widget_invalidate_force(widget, NULL);
    widget->state = tk_str_copy(widget->state, state);
    widget_set_need_update_style(widget);
    widget_invalidate_force(widget, NULL);
  }

  return RET_OK;
}

const char *widget_get_state_for_style(widget_t *widget, bool_t active, bool_t checked)
{
  const char *state = WIDGET_STATE_NORMAL;
  widget_t *iter = widget;
  return_value_if_fail(widget != NULL, state);

  state = (const char *)(widget->state);

  while (iter != NULL)
  {
    if (!iter->enable)
    {
      if (active)
        return WIDGET_STATE_DISABLE_OF_ACTIVE;
      if (checked)
        return WIDGET_STATE_DISABLE_OF_CHECKED;
      return WIDGET_STATE_DISABLE;
    }
    iter = iter->parent;
  }

  if (widget_is_focusable(widget) || widget_with_focus_state(widget))
  {
    if (widget->focused)
    {
      if (tk_str_eq(state, WIDGET_STATE_NORMAL))
      {
        state = WIDGET_STATE_FOCUSED;
      }
    }
    else
    {
      if (tk_str_eq(state, WIDGET_STATE_FOCUSED))
      {
        state = WIDGET_STATE_NORMAL;
      }
    }
  }

  if (active)
  {
    if (tk_str_eq(state, WIDGET_STATE_NORMAL))
    {
      state = WIDGET_STATE_NORMAL_OF_ACTIVE;
    }
    else if (tk_str_eq(state, WIDGET_STATE_PRESSED))
    {
      state = WIDGET_STATE_PRESSED_OF_ACTIVE;
    }
    else if (tk_str_eq(state, WIDGET_STATE_OVER))
    {
      state = WIDGET_STATE_OVER_OF_ACTIVE;
    }
    else if (widget_is_focusable(widget) && widget->focused)
    {
      state = WIDGET_STATE_FOCUSED_OF_ACTIVE;
    }
  }
  else if (checked)
  {
    if (tk_str_eq(state, WIDGET_STATE_NORMAL))
    {
      state = WIDGET_STATE_NORMAL_OF_CHECKED;
    }
    else if (tk_str_eq(state, WIDGET_STATE_PRESSED))
    {
      state = WIDGET_STATE_PRESSED_OF_CHECKED;
    }
    else if (tk_str_eq(state, WIDGET_STATE_OVER))
    {
      state = WIDGET_STATE_OVER_OF_CHECKED;
    }
    else if (widget_is_focusable(widget) && widget->focused)
    {
      state = WIDGET_STATE_FOCUSED_OF_CHECKED;
    }
  }

  return state;
}

ret_t widget_set_opacity(widget_t *widget, uint8_t opacity)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->opacity = opacity;
  widget_invalidate(widget, NULL);

  return RET_OK;
}

ret_t widget_set_dirty_rect_tolerance(widget_t *widget, uint16_t dirty_rect_tolerance)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->dirty_rect_tolerance = dirty_rect_tolerance;
  widget_invalidate(widget, NULL);

  return RET_OK;
}

ret_t widget_destroy_children(widget_t *widget)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->children != NULL)
  {
    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)

    widget_remove_child_prepare(widget, iter);
    widget_unref(iter);

    widget->children->elms[i] = NULL;

    WIDGET_FOR_EACH_CHILD_END();
    widget->children->size = 0;
  }

  return RET_OK;
}

const char *widget_get_style_type(widget_t *widget)
{
  theme_t *win_theme = NULL;
  theme_t *default_theme = NULL;
  const char *style_type = THEME_DEFAULT_STYLE_TYPE;

  if (widget_get_window_theme(widget, &win_theme, &default_theme) == RET_OK)
  {
    theme_t *t = win_theme != NULL ? win_theme : (default_theme != NULL ? default_theme : theme());
    if (t != NULL)
    {
      style_type = theme_get_style_type(t);
    }
  }

  return style_type;
}

static ret_t widget_update_style_object(widget_t *widget)
{
  const char *style_type = widget_get_style_type(widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);
  if (widget->astyle == NULL)
  {
    widget->astyle = style_factory_create_style(style_factory(), style_type);
    ENSURE(widget->astyle != NULL);

    if (widget_is_window_opened(widget))
    {
      widget_set_need_update_style(widget);
    }
  }
  else if (widget->astyle != NULL &&
           !tk_str_eq(style_get_style_type(widget->astyle), style_type))
  {
    style_t *style = style_factory_create_style(style_factory(), style_type);
    ENSURE(style != NULL);
    if (style_is_mutable(widget->astyle))
    {
      style_mutable_set_default_style(widget->astyle, style);
    }
    else
    {
      style_destroy(widget->astyle);
      widget->astyle = style;
    }

    if (widget_is_window_opened(widget))
    {
      widget_set_need_update_style(widget);
    }
  }
  return RET_OK;
}

static ret_t widget_update_style_object_recursive(widget_t *widget)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget_update_style_object(widget);
  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  widget_update_style_object_recursive(iter);
  WIDGET_FOR_EACH_CHILD_END();

  return RET_OK;
}

ret_t widget_add_child_default(widget_t *widget, widget_t *child)
{
  event_t e = event_init(EVT_WIDGET_ADD_CHILD, widget);
  return_value_if_fail(widget != NULL && child != NULL && child->parent == NULL, RET_BAD_PARAMS);

  child->parent = widget;

  if (child->vt->on_attach_parent)
  {
    child->vt->on_attach_parent(child, widget);
  }

  ENSURE(darray_push(widget->children, child) == RET_OK);

  if (!widget_is_window_manager(widget))
  {
    widget_set_need_relayout_children(widget);
  }

  if (!(child->initializing) && widget_get_window(child) != NULL)
  {
    widget_set_need_update_style_recursive(child);
    widget_update_style_object_recursive(child);
  }

  widget_dispatch(widget, &e);

  return RET_OK;
}

ret_t widget_add_child(widget_t *widget, widget_t *child)
{
  return_value_if_fail(widget != NULL && child != NULL && child->parent == NULL, RET_BAD_PARAMS);

  if (widget->children == NULL)
  {
    widget->children = darray_create(4, NULL, NULL);
  }

  if (widget->vt->on_add_child)
  {
    if (widget->vt->on_add_child(widget, child) == RET_OK)
    {
      return RET_OK;
    }
  }

  return widget_add_child_default(widget, child);
}

ret_t widget_remove_child_prepare(widget_t *widget, widget_t *child)
{
  return_value_if_fail(widget != NULL && child != NULL, RET_BAD_PARAMS);

  if (!widget_is_window_manager(widget))
  {
    widget_set_need_relayout_children(widget);
  }

  widget_invalidate_force(child, NULL);
  if (widget->target == child)
  {
    widget->target = NULL;
  }

  if (widget->grab_widget == child)
  {
    widget->grab_widget = NULL;
    widget->grab_widget_count = 0;
  }

  if (widget->key_target == child)
  {
    widget_dispatch_blur_event(widget->key_target);
    widget->key_target = NULL;
  }

  if (widget->vt->on_remove_child)
  {
    if (widget->vt->on_remove_child(widget, child) == RET_OK)
    {
      return RET_OK;
    }
  }

  if (child->vt->on_detach_parent)
  {
    child->vt->on_detach_parent(child, widget);
  }
  child->parent = NULL;

  return RET_OK;
}

ret_t widget_remove_child(widget_t *widget, widget_t *child)
{
  ret_t ret = RET_OK;
  event_t e = event_init(EVT_WIDGET_REMOVE_CHILD, widget);
  return_value_if_fail(widget != NULL && child != NULL, RET_BAD_PARAMS);

  widget_remove_child_prepare(widget, child);
  ret = darray_remove(widget->children, child);

  if (ret == RET_OK)
  {
    widget_dispatch(widget, &e);
  }

  return ret;
}

ret_t widget_insert_child(widget_t *widget, uint32_t index, widget_t *child)
{
  return_value_if_fail(widget != NULL && child != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget_add_child(widget, child) == RET_OK, RET_FAIL);

  return widget_restack(child, index);
}

ret_t widget_restack(widget_t *widget, uint32_t index)
{
  uint32_t i = 0;
  uint32_t nr = 0;
  int32_t old_index = 0;
  widget_t **children = NULL;
  return_value_if_fail(widget != NULL && widget->parent != NULL, RET_BAD_PARAMS);

  old_index = widget_index_of(widget);
  nr = widget_count_children(widget->parent);
  return_value_if_fail(old_index >= 0 && nr > 0, RET_BAD_PARAMS);

  if (index >= nr)
  {
    index = nr - 1;
  }

  if (index == old_index || nr == 1)
  {
    return RET_OK;
  }

  children = (widget_t **)(widget->parent->children->elms);
  if (index < old_index)
  {
    for (i = old_index; i > index; i--)
    {
      children[i] = children[i - 1];
    }
  }
  else
  {
    for (i = old_index; i < index; i++)
    {
      children[i] = children[i + 1];
    }
  }
  children[index] = widget;

  return RET_OK;
}

static widget_t *widget_lookup_child(widget_t *widget, const char *name)
{
  return_value_if_fail(widget != NULL && name != NULL, NULL);

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (iter->name != NULL && tk_str_eq(iter->name, name))
  {
    return iter;
  }
  WIDGET_FOR_EACH_CHILD_END()

  return NULL;
}

widget_t *widget_child(widget_t *widget, const char *path)
{
  return widget_lookup_child(widget, path);
}

widget_t *widget_get_focused_widget(widget_t *widget)
{
  widget_t *iter = NULL;
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(win != NULL, NULL);

  iter = win->key_target;
  for (iter = win->key_target; iter != NULL; iter = iter->key_target)
  {
    if (iter->focusable && iter->focused)
    {
      return iter;
    }

    if (iter->key_target == NULL)
    {
      return iter;
    }
  }

  return NULL;
}

static widget_t *widget_lookup_all(widget_t *widget, const char *name)
{
  return_value_if_fail(widget != NULL && name != NULL, NULL);

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (iter->name != NULL && tk_str_eq(iter->name, name))
  {
    return iter;
  }
  else
  {
    iter = widget_lookup_all(iter, name);
    if (iter != NULL)
    {
      return iter;
    }
  }
  WIDGET_FOR_EACH_CHILD_END();

  return NULL;
}

widget_t *widget_lookup(widget_t *widget, const char *name, bool_t recursive)
{
  if (recursive)
  {
    return widget_lookup_all(widget, name);
  }
  else
  {
    return widget_lookup_child(widget, name);
  }
}

static widget_t *widget_lookup_by_type_child(widget_t *widget, const char *type)
{
  return_value_if_fail(widget != NULL && type != NULL, NULL);

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (tk_str_eq(iter->vt->type, type))
  {
    return iter;
  }
  WIDGET_FOR_EACH_CHILD_END()

  return NULL;
}

static widget_t *widget_lookup_by_type_all(widget_t *widget, const char *type)
{
  return_value_if_fail(widget != NULL && type != NULL, NULL);

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (tk_str_eq(iter->vt->type, type))
  {
    return iter;
  }
  else
  {
    iter = widget_lookup_by_type_all(iter, type);
    if (iter != NULL)
    {
      return iter;
    }
  }
  WIDGET_FOR_EACH_CHILD_END();

  return NULL;
}

widget_t *widget_lookup_by_type(widget_t *widget, const char *type, bool_t recursive)
{
  if (recursive)
  {
    return widget_lookup_by_type_all(widget, type);
  }
  else
  {
    return widget_lookup_by_type_child(widget, type);
  }
}

static ret_t widget_set_visible_self(widget_t *widget, bool_t visible)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->visible != visible)
  {
    widget_invalidate_force(widget, NULL);
    widget->visible = visible;
    widget_set_need_update_style(widget);
    widget_invalidate_force(widget, NULL);
    widget_set_need_relayout_children(widget->parent);
  }

  return RET_OK;
}

ret_t widget_set_sensitive(widget_t *widget, bool_t sensitive)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->sensitive = sensitive;

  return RET_OK;
}

ret_t widget_set_visible_only(widget_t *widget, bool_t visible)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->visible = visible;

  return RET_OK;
}

ret_t widget_set_visible(widget_t *widget, bool_t visible, ...)
{
  return widget_set_visible_self(widget, visible);
}

widget_t *widget_find_target(widget_t *widget, xy_t x, xy_t y)
{
  widget_t *ret = NULL;
  return_value_if_fail(widget != NULL, NULL);

  if (widget->vt && widget->vt->find_target)
  {
    ret = widget->vt->find_target(widget, x, y);
  }
  else
  {
    ret = widget_find_target_default(widget, x, y);
  }

  return ret;
}

ret_t widget_on_event_before_children(widget_t *widget, event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  if (widget->vt && widget->vt->on_event_before_children)
  {
    ret = widget->vt->on_event_before_children(widget, e);
  }

  return ret;
}

static const char *widget_get_pointer_cursor(widget_t *widget)
{
  if (widget->pointer_cursor != NULL)
  {
    return widget->pointer_cursor;
  }
  else if (widget->vt->pointer_cursor != NULL)
  {
    return widget->vt->pointer_cursor;
  }

  return WIDGET_CURSOR_DEFAULT;
}

ret_t widget_update_pointer_cursor(widget_t *widget)
{
  widget_t *wm = widget_get_window_manager(widget);
  return_value_if_fail(wm != NULL, RET_BAD_PARAMS);

  return window_manager_set_cursor(wm, widget_get_pointer_cursor(widget));
}

ret_t widget_dispatch(widget_t *widget, event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  if (e->target == NULL)
  {
    e->target = widget;
  }

  if (widget->vt && widget->vt->on_event)
  {
    ret = widget->vt->on_event(widget, e);
  }
  else
  {
    ret = widget_on_event_default(widget, e);
  }

  if (ret != RET_STOP)
  {
    if (widget->emitter != NULL)
    {
      void *saved_target = e->target;

      e->target = widget;
      ret = emitter_dispatch(widget->emitter, e);
      e->target = saved_target;
    }
  }
  widget_unref(widget);

  return ret;
}

static ret_t dispatch_in_idle(const idle_info_t *info)
{
  event_t *e = (event_t *)(info->ctx);
  widget_t *widget = WIDGET(e->target);

  widget_dispatch(widget, e);
  widget_unref(widget);
  event_destroy(e);

  return RET_REMOVE;
}

ret_t widget_dispatch_async(widget_t *widget, event_t *e)
{
  event_t *evt = NULL;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(e->target == widget, RET_BAD_PARAMS);

  evt = event_clone(e);
  return_value_if_fail(evt != NULL, RET_OOM);

  widget_ref(widget);
  idle_add(dispatch_in_idle, evt);

  return RET_OK;
}

static ret_t widget_dispatch_callback(void *ctx, const void *data)
{
  widget_t *widget = WIDGET(data);

  return widget_dispatch(widget, (event_t *)ctx);
}

ret_t widget_dispatch_recursive(widget_t *widget, event_t *e)
{
  return widget_foreach(widget, widget_dispatch_callback, e);
}

uint32_t widget_on_with_tag(widget_t *widget, uint32_t type, event_func_t on_event, void *ctx,
                            uint32_t tag)
{
  return_value_if_fail(widget != NULL && on_event != NULL, RET_BAD_PARAMS);
  if (widget->emitter == NULL)
  {
    widget->emitter = emitter_create();
  }

  return emitter_on_with_tag(widget->emitter, type, on_event, ctx, tag);
}

uint32_t widget_on(widget_t *widget, uint32_t type, event_func_t on_event, void *ctx)
{
  return widget_on_with_tag(widget, type, on_event, ctx, 0);
}

uint32_t widget_child_on(widget_t *widget, const char *name, uint32_t type, event_func_t on_event,
                         void *ctx)
{
  return widget_on(widget_lookup(widget, name, TRUE), type, on_event, ctx);
}

ret_t widget_off(widget_t *widget, uint32_t id)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->emitter != NULL, RET_BAD_PARAMS);

  return emitter_off(widget->emitter, id);
}

ret_t widget_off_by_tag(widget_t *widget, uint32_t tag)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->emitter == NULL)
  {
    return RET_OK;
  }

  return emitter_off_by_tag(widget->emitter, tag);
}

ret_t widget_off_by_ctx(widget_t *widget, void *ctx)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->emitter == NULL)
  {
    return RET_OK;
  }

  return emitter_off_by_ctx(widget->emitter, ctx);
}

ret_t widget_off_by_func(widget_t *widget, uint32_t type, event_func_t on_event, void *ctx)
{
  return_value_if_fail(widget != NULL && on_event != NULL, RET_BAD_PARAMS);

  if (widget->emitter == NULL)
  {
    return RET_OK;
  }

  return emitter_off_by_func(widget->emitter, type, on_event, ctx);
}

ret_t widget_calc_icon_text_rect(const rect_t *ir, int32_t font_size, float_t text_size,
                                 int32_t icon_at, uint32_t img_w, uint32_t img_h, int32_t spacer,
                                 rect_t *r_text, rect_t *r_icon)
{
  return_value_if_fail(ir != NULL && (r_text != NULL || r_icon != NULL), RET_BAD_PARAMS);

  if (r_icon == NULL)
  {
    *r_text = *ir;

    return RET_OK;
  }

  if (r_text == NULL)
  {
    *r_icon = *ir;

    return RET_OK;
  }

  return_value_if_fail(spacer < ir->h && spacer < ir->w, RET_BAD_PARAMS);
  switch (icon_at)
  {
  case ICON_AT_CENTRE:
  {
    int32_t w = ir->w - spacer - text_size - img_w;
    int32_t icon_h = ir->h - img_h;
    *r_text = rect_init(ir->x + img_w + spacer + w / 2, ir->y, text_size, ir->h);
    *r_icon = rect_init(ir->x + w / 2, ir->y + icon_h / 2, img_w, img_h);
    break;
  }
  case ICON_AT_RIGHT:
  {
    uint32_t w = img_w;
    float_t ratio = system_info()->device_pixel_ratio;
    if (ratio > 1)
    {
      w = img_w / ratio;
    }
    w = tk_min(tk_max(w, ir->h), ir->w);
    *r_icon = rect_init(ir->x + ir->w - w, ir->y, w, ir->h);
    *r_text = rect_init(ir->x, ir->y, ir->w - ir->h - spacer, ir->h);
    break;
  }
  case ICON_AT_TOP:
  {
    int32_t icon_h = ir->h - font_size - spacer;
    *r_icon = rect_init(ir->x, ir->y, ir->w, icon_h);
    *r_text = rect_init(ir->x, icon_h + spacer, ir->w, font_size);
    break;
  }
  case ICON_AT_BOTTOM:
  {
    int32_t icon_h = ir->h - font_size - spacer;
    *r_icon = rect_init(ir->x, ir->y + ir->h - icon_h, ir->w, icon_h);
    *r_text = rect_init(ir->x, ir->y, ir->w, font_size);
    break;
  }
  case ICON_AT_LEFT:
  default:
  {
    *r_icon = rect_init(ir->x, ir->y, ir->h, ir->h);
    *r_text = rect_init(ir->x + ir->h + spacer, ir->y, ir->w - ir->h - spacer, ir->h);
    break;
  }
  }

  return RET_OK;
}

const char *widget_get_bidi(widget_t *widget)
{
  value_t v;
  if (widget_get_prop(widget, WIDGET_PROP_BIDI, &v) == RET_OK)
  {
    return value_str(&v);
  }

  return NULL;
}

ret_t widget_draw_icon_text(widget_t *widget, canvas_t *c, const char *icon, wstr_t *text)
{
  rect_t ir;
  wh_t w = 0;
  wh_t h = 0;
  bitmap_t img;
  rect_t r_icon;
  rect_t r_text;
  int32_t margin = 0;
  int32_t spacer = 0;
  int32_t icon_at = 0;
  uint16_t font_size = 0;
  float_t text_size = 0.0f;
  int32_t margin_left = 0;
  int32_t margin_right = 0;
  int32_t margin_top = 0;
  int32_t margin_bottom = 0;
  style_t *style = widget->astyle;
  int32_t align_h = ALIGN_H_LEFT;
  int32_t align_v = ALIGN_V_MIDDLE;
  return_value_if_fail(widget->astyle != NULL, RET_BAD_PARAMS);

  spacer = style_get_int(style, STYLE_ID_SPACER, 2);
  margin = style_get_int(style, STYLE_ID_MARGIN, 0);
  margin_top = style_get_int(style, STYLE_ID_MARGIN_TOP, margin);
  margin_left = style_get_int(style, STYLE_ID_MARGIN_LEFT, margin);
  margin_right = style_get_int(style, STYLE_ID_MARGIN_RIGHT, margin);
  margin_bottom = style_get_int(style, STYLE_ID_MARGIN_BOTTOM, margin);
  icon_at = style_get_int(style, STYLE_ID_ICON_AT, ICON_AT_AUTO);

  w = widget->w - margin_left - margin_right;
  h = widget->h - margin_top - margin_bottom;
  ir = rect_init(margin_left, margin_top, w, h);

  if (text == NULL)
  {
    text = &(widget->text);
  }

  if (icon == NULL)
  {
    icon = style_get_str(style, STYLE_ID_ICON, NULL);
  }

  widget_prepare_text_style(widget, c);

  font_size = c->font_size;
  text_size = text->str ? canvas_measure_text(c, text->str, text->size) : 0;
  if (icon_at == ICON_AT_RIGHT || icon_at == ICON_AT_LEFT)
  {
    align_v = style_get_int(style, STYLE_ID_TEXT_ALIGN_V, ALIGN_V_MIDDLE);
    align_h = style_get_int(style, STYLE_ID_TEXT_ALIGN_H, ALIGN_H_LEFT);
  }
  else
  {
    align_v = style_get_int(style, STYLE_ID_TEXT_ALIGN_V, ALIGN_V_MIDDLE);
    align_h = style_get_int(style, STYLE_ID_TEXT_ALIGN_H, ALIGN_H_CENTER);
  }
  canvas_set_text_align(c, (align_h_t)align_h, (align_v_t)align_v);

  if (icon != NULL && widget_load_image(widget, icon, &img) == RET_OK)
  {
    float_t dpr = system_info()->device_pixel_ratio;

    if (text->size > 0)
    {
      if ((h > (img.h / dpr + font_size) && icon_at == ICON_AT_AUTO))
      {
        icon_at = ICON_AT_TOP;
      }

      widget_calc_icon_text_rect(&ir, font_size, text_size, icon_at, img.w, img.h, spacer, &r_text,
                                 &r_icon);

      canvas_draw_icon_in_rect(c, &img, &r_icon);
      widget_draw_text_in_rect(widget, c, text->str, text->size, &r_text, FALSE);
    }
    else
    {
      if (icon_at == ICON_AT_AUTO)
      {
        widget_calc_icon_text_rect(&ir, font_size, text_size, icon_at, img.w, img.h, spacer, NULL,
                                   &r_icon);
      }
      else
      {
        widget_calc_icon_text_rect(&ir, font_size, text_size, icon_at, img.w, img.h, spacer,
                                   &r_text, &r_icon);
      }
      canvas_draw_icon_in_rect(c, &img, &r_icon);
    }
  }
  else if (text != NULL && text->size > 0)
  {
    widget_calc_icon_text_rect(&ir, font_size, text_size, icon_at, 0, 0, spacer, &r_text, NULL);
    widget_draw_text_in_rect(widget, c, text->str, text->size, &r_text, FALSE);
  }

  return RET_OK;
}

ret_t widget_fill_rect(widget_t *widget, canvas_t *c, const rect_t *r, bool_t bg,
                       image_draw_type_t draw_type)
{
  bitmap_t img;
  ret_t ret = RET_OK;
  gradient_t agradient;
  style_t *style = widget->astyle;
  uint32_t radius = style_get_int(style, STYLE_ID_ROUND_RADIUS, 0);
  const char *color_key = bg ? STYLE_ID_BG_COLOR : STYLE_ID_FG_COLOR;
  const char *image_key = bg ? STYLE_ID_BG_IMAGE : STYLE_ID_FG_IMAGE;
  rect_t bg_r = rect_init(widget->x, widget->y, widget->w, widget->h);
  uint32_t radius_tl = style_get_int(style, STYLE_ID_ROUND_RADIUS_TOP_LEFT, radius);
  uint32_t radius_tr = style_get_int(style, STYLE_ID_ROUND_RADIUS_TOP_RIGHT, radius);
  uint32_t radius_bl = style_get_int(style, STYLE_ID_ROUND_RADIUS_BOTTOM_LEFT, radius);
  uint32_t radius_br = style_get_int(style, STYLE_ID_ROUND_RADIUS_BOTTOM_RIGHT, radius);
  uint32_t clear_bg = style_get_uint(style, STYLE_ID_CLEAR_BG, 0);
  const char *draw_type_key = bg ? STYLE_ID_BG_IMAGE_DRAW_TYPE : STYLE_ID_FG_IMAGE_DRAW_TYPE;
  gradient_t *gradient = style_get_gradient(style, color_key, &agradient);
  const char *image_name = style_get_str(style, image_key, NULL);

  if (gradient != NULL && r->w > 0 && r->h > 0)
  {
    color_t color = gradient_get_first_color(gradient);
    canvas_set_fill_color(c, color);
    if (gradient->nr > 1 || color.rgba.a)
    {
      if (radius_tl > 3 || radius_tr > 3 || radius_bl > 3 || radius_br > 3)
      {
        /*TODO: support gradient*/
        if (bg)
        {
          ret = canvas_fill_rounded_rect_gradient_ex(c, r, NULL, gradient, radius_tl, radius_tr,
                                                     radius_bl, radius_br);
        }
        else
        {
          ret = canvas_fill_rounded_rect_gradient_ex(c, r, &bg_r, gradient, radius_tl, radius_tr,
                                                     radius_bl, radius_br);
        }
        if (ret == RET_FAIL)
        {
          canvas_fill_rect(c, r->x, r->y, r->w, r->h);
        }
      }
      else if (gradient->nr > 1)
      {
        canvas_fill_rect_gradient(c, r->x, r->y, r->w, r->h, gradient);
      }
      else
      {
        if (clear_bg)
        {
          canvas_clear_rect(c, r->x, r->y, r->w, r->h);
        }
        else
        {
          canvas_fill_rect(c, r->x, r->y, r->w, r->h);
        }
      }
    }
    else if (clear_bg)
    {
      canvas_clear_rect(c, r->x, r->y, r->w, r->h);
    }
  }

  if (image_name != NULL && *image_name && r->w > 0 && r->h > 0)
  {
    char name[MAX_PATH + 1];
    const char *region = strrchr(image_name, '#');
    if (region != NULL)
    {
      memset(name, 0x00, sizeof(name));
      tk_strncpy(name, image_name, region - image_name);
      image_name = name;
    }

    if (widget_load_image(widget, image_name, &img) == RET_OK)
    {
      draw_type = (image_draw_type_t)style_get_int(style, draw_type_key, draw_type);

      if (region == NULL)
      {
        canvas_draw_image_ex(c, &img, draw_type, r);
      }
      else
      {
        rect_t src;
        rect_t dst = *r;
        if (tk_str_eq(region, "#"))
        {
          src = rect_init(widget->x, widget->y, widget->w, widget->h);
        }
        else if (tk_str_eq(region, "#g"))
        {
          point_t p = {widget->x, widget->y};
          widget_to_global(widget, &p);
          src = rect_init(p.x, p.y, widget->w, widget->h);
        }
        else
        {
          image_region_parse(img.w, img.h, region, &src);
        }

        canvas_draw_image_ex2(c, &img, draw_type, &src, &dst);
      }
    }
  }

  return RET_OK;
}

ret_t widget_stroke_border_rect_for_border_type(canvas_t *c, const rect_t *r, color_t bd,
                                                int32_t border, uint32_t border_width)
{
  wh_t w = r->w;
  wh_t h = r->h;
  xy_t x = r->x + 0.5;
  xy_t y = r->y + 0.5;
  xy_t y1 = y;
  wh_t h1 = h;
  bool_t draw_top = FALSE;
  bool_t draw_bottom = FALSE;
  canvas_set_fill_color(c, bd);
  if (border & BORDER_TOP)
  {
    draw_top = TRUE;
    canvas_fill_rect(c, x, y, w, border_width);
  }
  if (border & BORDER_BOTTOM)
  {
    draw_bottom = TRUE;
    canvas_fill_rect(c, x, y + h - border_width, w, border_width);
  }
  /* 减少重复绘制的部分，可以修复有透明的时候重叠区域显示不正常为问题 */
  if (draw_top)
  {
    y1 += border_width;
    h1 -= border_width;
  }
  if (draw_bottom)
  {
    h1 -= border_width;
  }
  if (border & BORDER_LEFT)
  {
    canvas_fill_rect(c, x, y1, border_width, h1);
  }
  if (border & BORDER_RIGHT)
  {
    canvas_fill_rect(c, x + w - border_width, y1, border_width, h1);
  }
  return RET_OK;
}

ret_t widget_stroke_border_rect(widget_t *widget, canvas_t *c, const rect_t *r)
{
  style_t *style = widget->astyle;
  color_t trans = color_init(0, 0, 0, 0);
  color_t bd = style_get_color(style, STYLE_ID_BORDER_COLOR, trans);
  uint32_t radius = style_get_int(style, STYLE_ID_ROUND_RADIUS, 0);
  int32_t border = style_get_int(style, STYLE_ID_BORDER, BORDER_ALL);
  uint32_t border_width = style_get_int(style, STYLE_ID_BORDER_WIDTH, 1);
  uint32_t radius_tl = style_get_int(style, STYLE_ID_ROUND_RADIUS_TOP_LEFT, radius);
  uint32_t radius_tr = style_get_int(style, STYLE_ID_ROUND_RADIUS_TOP_RIGHT, radius);
  uint32_t radius_bl = style_get_int(style, STYLE_ID_ROUND_RADIUS_BOTTOM_LEFT, radius);
  uint32_t radius_br = style_get_int(style, STYLE_ID_ROUND_RADIUS_BOTTOM_RIGHT, radius);

  if (bd.rgba.a)
  {
    canvas_set_stroke_color(c, bd);
    if (radius_tl > 3 || radius_tr > 3 || radius_bl > 3 || radius_br > 3)
    {
      if (canvas_stroke_rounded_rect_ex(c, r, NULL, &bd, radius_tl, radius_tr, radius_bl, radius_br,
                                        border_width, border) != RET_OK)
      {
        widget_stroke_border_rect_for_border_type(c, r, bd, border, border_width);
      }
    }
    else
    {
      widget_stroke_border_rect_for_border_type(c, r, bd, border, border_width);
    }
  }

  return RET_OK;
}

ret_t widget_draw_background(widget_t *widget, canvas_t *c)
{
  rect_t r;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);

  r = rect_init(0, 0, widget->w, widget->h);

  return widget_fill_rect(widget, c, &r, TRUE, IMAGE_DRAW_CENTER);
}

ret_t widget_fill_bg_rect(widget_t *widget, canvas_t *c, const rect_t *r,
                          image_draw_type_t draw_type)
{
  return widget_fill_rect(widget, c, r, TRUE, draw_type);
}

ret_t widget_fill_fg_rect(widget_t *widget, canvas_t *c, const rect_t *r,
                          image_draw_type_t draw_type)
{
  return widget_fill_rect(widget, c, r, FALSE, draw_type);
}

static ret_t widget_draw_border(widget_t *widget, canvas_t *c)
{
  rect_t r;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);

  r = rect_init(0, 0, widget->w, widget->h);
  return widget_stroke_border_rect(widget, c, &r);
}

ret_t widget_paint_helper(widget_t *widget, canvas_t *c, const char *icon, wstr_t *text)
{
  if (style_is_valid(widget->astyle))
  {
    widget_draw_icon_text(widget, c, icon, text);
  }

  return RET_OK;
}

static ret_t widget_paint_impl(widget_t *widget, canvas_t *c)
{
  int32_t ox = widget->x;
  int32_t oy = widget->y;
  uint8_t save_alpha = c->global_alpha;

  if (widget->opacity < TK_OPACITY_ALPHA)
  {
    canvas_set_global_alpha(c, (widget->opacity * save_alpha) / 0xff);
  }

  if (widget->astyle != NULL)
  {
    ox += style_get_int(widget->astyle, STYLE_ID_X_OFFSET, 0);
    oy += style_get_int(widget->astyle, STYLE_ID_Y_OFFSET, 0);
  }

  canvas_translate(c, ox, oy);
  widget_on_paint_begin(widget, c);
  widget_on_paint_background(widget, c);
  widget_on_paint_self(widget, c);
  widget_on_paint_children(widget, c);
  widget_on_paint_border(widget, c);
  widget_on_paint_end(widget, c);

  canvas_untranslate(c, ox, oy);
  if (widget->opacity < TK_OPACITY_ALPHA)
  {
    canvas_set_global_alpha(c, save_alpha);
  }

  widget_on_paint_done(widget, c);

  return RET_OK;
}

ret_t widget_paint(widget_t *widget, canvas_t *c)
{
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);

  if (!widget->visible || widget->opacity <= 0x08 || widget->w <= 0 || widget->h <= 0)
  {
    widget->dirty = FALSE;
    return RET_OK;
  }

  if (widget->need_update_style)
  {
    widget_update_style(widget);
  }

  canvas_save(c);
  widget_paint_impl(widget, c);
  canvas_restore(c);

  widget->dirty = FALSE;

  return RET_OK;
}

static const widget_cmd_t s_widget_cmds[] = {
    {WIDGET_EXEC_START_ANIMATOR, widget_start_animator},
    {WIDGET_EXEC_STOP_ANIMATOR, widget_stop_animator},
    {WIDGET_EXEC_PAUSE_ANIMATOR, widget_pause_animator},
    {WIDGET_EXEC_DESTROY_ANIMATOR, widget_destroy_animator}};

static ret_t widget_do_exec(widget_t *widget, const char *cmd, const char *args)
{
  uint32_t i = 0;

  for (i = 0; i < ARRAY_SIZE(s_widget_cmds); i++)
  {
    const widget_cmd_t *iter = s_widget_cmds + i;
    if (tk_str_eq(cmd, iter->name))
    {
      return iter->exec(widget, args);
    }
  }

  return RET_NOT_FOUND;
}

static ret_t widget_exec(widget_t *widget, const char *str)
{
  if (str != NULL)
  {
    char cmd[TK_NAME_LEN + 1] = {0};
    const char *args = strchr(str, ':');

    if (args != NULL)
    {
      return_value_if_fail((args - str) < TK_NAME_LEN, RET_BAD_PARAMS);
      tk_strncpy(cmd, str, args - str);
      args += 1;
    }
    else
    {
      return_value_if_fail(strlen(str) < TK_NAME_LEN, RET_BAD_PARAMS);
      tk_strcpy(cmd, str);
    }

    return widget_do_exec(widget, cmd, args);
  }
  else
  {
    return RET_NOT_FOUND;
  }
}

static widget_t *widget_get_top_widget_grab_key(widget_t *widget)
{
  return_value_if_fail(widget != NULL, NULL);
  WIDGET_FOR_EACH_CHILD_BEGIN_R(widget, iter, i)
  value_t v;
  widget_t *widget_grab_key = widget_get_top_widget_grab_key(iter);
  if (widget_grab_key == NULL && iter != NULL && iter->visible && iter->custom_props != NULL)
  {
    ret_t ret = tk_object_get_prop(iter->custom_props, WIDGET_PROP_GRAB_KEYS, &v);
    if (ret == RET_OK && value_bool(&v))
    {
      return iter;
    }
  }
  WIDGET_FOR_EACH_CHILD_END();

  return NULL;
}

static ret_t widget_on_ungrab_keys(void *ctx, event_t *e)
{
  widget_t *widget = WIDGET(ctx);
  window_manager_t *wm = WINDOW_MANAGER(widget_get_window_manager(widget));

  wm->widget_grab_key = widget_get_top_widget_grab_key(WIDGET(wm));

  return RET_REMOVE;
}

#ifndef WITHOUT_FSCRIPT
typedef struct _fscript_info_t
{
  char *code;
  fscript_t *fscript;
  widget_t *widget;
  tk_object_t *obj;
  emitter_t *emitter;
  bool_t is_global_object;
  bool_t busy;
} fscript_info_t;

static fscript_info_t *fscript_info_create(const char *code, widget_t *widget)
{
  fscript_info_t *info = NULL;
  return_value_if_fail(code != NULL && widget != NULL, NULL);
  info = TKMEM_ZALLOC(fscript_info_t);
  return_value_if_fail(info != NULL, NULL);

  info->widget = widget;
  info->code = tk_strdup(code);

  if (info->code == NULL)
  {
    TKMEM_FREE(info);
  }

  return info;
}

static ret_t fscript_info_destroy(fscript_info_t *info)
{
  return_value_if_fail(info != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(info->code);
  if (info->fscript != NULL)
  {
    fscript_destroy(info->fscript);
    info->fscript = NULL;
  }

  if (info->emitter != NULL)
  {
    emitter_off_by_ctx(info->emitter, info);
  }
  else
  {
    widget_off_by_ctx(info->widget, info);
  }

  TK_OBJECT_UNREF(info->obj);
  TKMEM_FREE(info);

  return RET_OK;
}

static ret_t fscript_info_prepare(fscript_info_t *info, event_t *evt)
{
  tk_object_t *obj = NULL;

  if (info->fscript == NULL)
  {
    value_t v;
    tk_object_t *model = NULL;

    if (widget_get_prop(info->widget, STR_PROP_MODEL, &v) == RET_OK)
    {
      model = value_object(&v);
    }
    obj = object_default_create();
    return_value_if_fail(obj != NULL, RET_BAD_PARAMS);

    if (model != NULL)
    {
      tk_object_set_prop_object(obj, STR_PROP_MODEL, model);
    }
    tk_object_set_prop_pointer(obj, STR_PROP_SELF, info->widget);

    info->obj = obj;
    info->fscript = fscript_create(obj, info->code);
    return_value_if_fail(info->fscript != NULL, RET_BAD_PARAMS);
    TKMEM_FREE(info->code);
  }
  else
  {
    obj = info->obj;
  }

  if (evt == NULL)
  {
    return RET_OK;
  }

  switch (evt->type)
  {
  case EVT_CLICK:
  case EVT_POINTER_DOWN:
  case EVT_POINTER_MOVE:
  case EVT_POINTER_UP:
  {
    pointer_event_t *e = pointer_event_cast(evt);
    tk_object_set_prop_int(obj, "x", e->x);
    tk_object_set_prop_int(obj, "y", e->y);
    tk_object_set_prop_bool(obj, "alt", e->alt);
    tk_object_set_prop_bool(obj, "cmd", e->cmd);
    tk_object_set_prop_bool(obj, "menu", e->menu);
    tk_object_set_prop_bool(obj, "ctrl", e->ctrl);
    break;
  }
  case EVT_KEY_DOWN:
  case EVT_KEY_LONG_PRESS:
  case EVT_KEY_UP:
  {
    key_event_t *e = key_event_cast(evt);
    const key_type_value_t *kv = keys_type_find_by_value(e->key);
    if (kv != NULL)
    {
      tk_object_set_prop_str(obj, "key", kv->name);
    }
    else
    {
      tk_object_set_prop_str(obj, "key", "unkown");
    }
    tk_object_set_prop_bool(obj, "alt", e->alt);
    tk_object_set_prop_bool(obj, "cmd", e->cmd);
    tk_object_set_prop_bool(obj, "menu", e->menu);
    tk_object_set_prop_bool(obj, "ctrl", e->ctrl);
    break;
  }
  case EVT_MODEL_CHANGE:
  {
    model_event_t *e = model_event_cast(evt);
    tk_object_set_prop_str(obj, "name", e->name);
    tk_object_set_prop_str(obj, "change_type", e->change_type);
    tk_object_set_prop_object(obj, "model", e->model);
    break;
  }
  default:
    break;
  }

  return RET_OK;
}

static ret_t fscript_info_exec(fscript_info_t *info)
{
  value_t result;
  ret_t ret = RET_OK;
  value_set_int(&result, RET_OK);
  if (fscript_exec(info->fscript, &result) == RET_OK)
  {
    value_t v;
    tk_object_t *obj = info->obj;

    if (tk_object_get_prop(obj, "RET_STOP", &v) == RET_OK && value_bool(&v))
    {
      ret = RET_STOP;
    }

    if (tk_object_get_prop(obj, "RET_REMOVE", &v) == RET_OK && value_bool(&v))
    {
      ret = RET_REMOVE;
    }

    if (ret == RET_OK)
    {
      if (result.type == VALUE_TYPE_UINT32 || result.type == VALUE_TYPE_INT32)
      {
        ret = value_int(&result);
      }
    }

    value_reset(&result);
  }

  return ret;
}

extern bool_t tk_is_ui_thread(void);

static ret_t widget_exec_code(void *ctx, event_t *evt)
{
  ret_t ret = RET_OK;
  fscript_info_t *info = (fscript_info_t *)ctx;

  if (info->busy)
  {
    /*多次触发，只执行一次*/
    return RET_OK;
  }

  if (!tk_is_ui_thread())
  {
    log_debug("not supported trigger in none ui thread\n");
    return RET_OK;
  }

  info->busy = TRUE;
  return_value_if_fail(fscript_info_prepare(info, evt) == RET_OK, RET_BAD_PARAMS);
  ret = fscript_info_exec(info);
  info->busy = FALSE;

  return ret;
}

static ret_t widget_free_code(void *ctx, event_t *evt)
{
  widget_t *widget = WIDGET(evt->target);
  widget_off_by_ctx(widget, ctx);
  TKMEM_FREE(ctx);

  return RET_REMOVE;
}
#endif /*WITHOUT_FSCRIPT*/

ret_t widget_set_prop(widget_t *widget, const char *name, const value_t *v)
{
  ret_t ret = RET_OK;
  prop_change_event_t e;
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_EXEC))
  {
    ret = widget_exec(widget, value_str(v));
    if (ret != RET_NOT_FOUND)
    {
      return ret;
    }
  }

  e.value = v;
  e.name = name;
  e.e = event_init(EVT_PROP_WILL_CHANGE, widget);
  widget_dispatch(widget, (event_t *)&e);

  if (tk_str_eq(name, WIDGET_PROP_X))
  {
    widget_set_x(widget, (xy_t)value_int(v), TRUE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_Y))
  {
    widget_set_y(widget, (xy_t)value_int(v), TRUE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_W))
  {
    widget_set_w(widget, (wh_t)value_int(v), TRUE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_H))
  {
    widget_set_h(widget, (wh_t)value_int(v), TRUE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_OPACITY))
  {
    widget->opacity = (uint8_t)value_int(v);
  }
  else if (tk_str_eq(name, WIDGET_PROP_VISIBLE))
  {
    widget_set_visible(widget, value_bool(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_SENSITIVE))
  {
    widget->sensitive = value_bool(v);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FLOATING))
  {
    widget->floating = value_bool(v);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FOCUSABLE))
  {
    widget->focusable = value_bool(v);
  }
  else if (tk_str_eq(name, WIDGET_PROP_WITH_FOCUS_STATE))
  {
    widget->with_focus_state = value_bool(v);
  }
  else if (tk_str_eq(name, WIDGET_PROP_DIRTY_RECT_TOLERANCE))
  {
    widget->dirty_rect_tolerance = value_int(v);
  }
  else if (tk_str_eq(name, WIDGET_PROP_STYLE))
  {
    const char *name = value_str(v);
    return widget_use_style(widget, name);
  }
  else if (tk_str_eq(name, WIDGET_PROP_ENABLE))
  {
    widget_set_enable(widget, value_bool(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_FEEDBACK))
  {
    widget->feedback = value_bool(v);
  }
  else if (tk_str_eq(name, WIDGET_PROP_AUTO_ADJUST_SIZE))
  {
    widget_set_auto_adjust_size(widget, value_bool(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_NAME))
  {
    widget_set_name(widget, value_str(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_TR_TEXT))
  {
    widget_set_tr_text(widget, value_str(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_ANIMATION))
  {
    widget_set_animation(widget, value_str(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_SELF_LAYOUT))
  {
    widget_set_self_layout(widget, value_str(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_LAYOUT) || tk_str_eq(name, WIDGET_PROP_CHILDREN_LAYOUT))
  {
    widget_set_children_layout(widget, value_str(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_POINTER_CURSOR))
  {
    widget_set_pointer_cursor(widget, value_str(v));
  }
  else
  {
    ret = RET_NOT_FOUND;
  }

  if (tk_str_start_with(name, WIDGET_PROP_ANIMATE_PREFIX))
  {
    uint32_t duration = TK_ANIMATING_TIME;
    const char *prop_name = name + strlen(WIDGET_PROP_ANIMATE_PREFIX);
    if (!tk_str_eq(prop_name, WIDGET_PROP_ANIMATING_TIME))
    {
      duration = widget_get_prop_int(widget, WIDGET_PROP_ANIMATE_ANIMATING_TIME, TK_ANIMATING_TIME);
      return widget_animate_prop_float_to(widget, prop_name, value_float32(v), duration);
    }
  }

  if (widget->vt->set_prop)
  {
    ret_t ret1 = widget->vt->set_prop(widget, name, v);
    if (ret == RET_NOT_FOUND)
    {
      ret = ret1;
    }
  }

  if (ret == RET_NOT_FOUND)
  {
    if (tk_str_eq(name, WIDGET_PROP_FOCUSED) || tk_str_eq(name, WIDGET_PROP_FOCUS))
    {
      widget_set_focused(widget, value_bool(v));
      ret = RET_OK;
    }
    else if (tk_str_eq(name, WIDGET_PROP_TEXT))
    {
      wstr_from_value(&(widget->text), v);
      ret = RET_OK;
    }
    else if (tk_str_eq(name, WIDGET_PROP_EXEC))
    {
      ret = RET_NOT_FOUND;
    }
    else if (tk_str_start_with(name, "style:") || tk_str_start_with(name, "style."))
    {
      return widget_set_style(widget, name + 6, v);
    }
    else
    {
      if (widget->custom_props == NULL)
      {
        widget->custom_props = object_default_create();
      }

      if (tk_str_eq(name, WIDGET_PROP_GRAB_KEYS))
      {
        window_manager_t *wm = WINDOW_MANAGER(widget_get_window_manager(widget));

        if (value_bool(v))
        {
          widget_on(widget, EVT_DESTROY, widget_on_ungrab_keys, widget);
          wm->widget_grab_key = widget;
        }
      }

#ifndef WITHOUT_FSCRIPT
      if (strncmp(name, STR_ON_EVENT_PREFIX, sizeof(STR_ON_EVENT_PREFIX) - 1) == 0)
      {
        bool_t is_global_vars_changed =
            tk_str_eq(name, STR_ON_EVENT_PREFIX "" STR_GLOBAL_VARS_CHANGED);
        int32_t etype = is_global_vars_changed
                            ? EVT_PROP_CHANGED
                            : event_from_name(name + sizeof(STR_ON_EVENT_PREFIX) - 1);
        if (etype != EVT_NONE)
        {
          fscript_info_t *info = fscript_info_create(value_str(v), widget);
          if (info != NULL)
          {
            char new_prop_name[TK_NAME_LEN + 1] = {0};
            name += sizeof(STR_ON_EVENT_PREFIX) - 1;

            if (is_global_vars_changed)
            {
              tk_object_t *global = fscript_get_global_object();
              emitter_on(EMITTER(global), EVT_PROPS_CHANGED, widget_exec_code, info);
              emitter_on(EMITTER(global), EVT_PROP_CHANGED, widget_exec_code, info);
              info->emitter = EMITTER(global);
            }
            else if (tk_str_start_with(name, STR_GLOBAL_EVENT_PREFIX))
            {
              widget_t *wm = window_manager();
              widget_on(wm, etype, widget_exec_code, info);
              info->emitter = EMITTER(wm->emitter);
            }
            else
            {
              widget_on(widget, etype, widget_exec_code, info);
              info->emitter = NULL;
            }
            tk_snprintf(new_prop_name, sizeof(new_prop_name) - 1, "%s-fscript-info", name);
            widget_set_prop_pointer_ex(widget, new_prop_name, info,
                                       (tk_destroy_t)fscript_info_destroy);
            ret = RET_OK;
          }
        }
        else
        {
          log_debug("not found event %s\n", name);
        }
      }
      else
#endif /*WITHOUT_FSCRIPT*/
      {
        ret = tk_object_set_prop(widget->custom_props, name, v);
      }
    }
  }

  if (ret != RET_NOT_FOUND)
  {
    e.e.type = EVT_PROP_CHANGED;
    widget_dispatch(widget, (event_t *)&e);
    widget_invalidate(widget, NULL);
  }

  return ret;
}

ret_t widget_get_prop(widget_t *widget, const char *name, value_t *v)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_X))
  {
    value_set_int32(v, widget->x);
  }
  else if (tk_str_eq(name, WIDGET_PROP_Y))
  {
    value_set_int32(v, widget->y);
  }
  else if (tk_str_eq(name, WIDGET_PROP_W))
  {
    value_set_int32(v, widget->w);
  }
  else if (tk_str_eq(name, WIDGET_PROP_H))
  {
    value_set_int32(v, widget->h);
  }
  else if (tk_str_eq(name, WIDGET_PROP_OPACITY))
  {
    value_set_int32(v, widget->opacity);
  }
  else if (tk_str_eq(name, WIDGET_PROP_VISIBLE))
  {
    value_set_bool(v, widget->visible);
  }
  else if (tk_str_eq(name, WIDGET_PROP_SENSITIVE))
  {
    value_set_bool(v, widget->sensitive);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FLOATING))
  {
    value_set_bool(v, widget->floating);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FOCUSABLE))
  {
    value_set_bool(v, widget_is_focusable(widget));
  }
  else if (tk_str_eq(name, WIDGET_PROP_FOCUSED))
  {
    value_set_bool(v, widget->focused);
  }
  else if (tk_str_eq(name, WIDGET_PROP_WITH_FOCUS_STATE))
  {
    value_set_bool(v, widget->with_focus_state);
  }
  else if (tk_str_eq(name, WIDGET_PROP_DIRTY_RECT_TOLERANCE))
  {
    value_set_int(v, widget->dirty_rect_tolerance);
  }
  else if (tk_str_eq(name, WIDGET_PROP_STYLE))
  {
    value_set_str(v, widget->style);
  }
  else if (tk_str_eq(name, WIDGET_PROP_ENABLE))
  {
    value_set_bool(v, widget->enable);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FEEDBACK))
  {
    value_set_bool(v, widget->feedback);
  }
  else if (tk_str_eq(name, WIDGET_PROP_AUTO_ADJUST_SIZE))
  {
    value_set_bool(v, widget->auto_adjust_size);
  }
  else if (tk_str_eq(name, WIDGET_PROP_NAME))
  {
    value_set_str(v, widget->name);
  }
  else if (tk_str_eq(name, WIDGET_PROP_ANIMATION))
  {
    value_set_str(v, widget->animation);
  }
  else if (tk_str_eq(name, WIDGET_PROP_POINTER_CURSOR))
  {
    value_set_str(v, widget->pointer_cursor);
  }
  else if (tk_str_eq(name, WIDGET_PROP_LOADING))
  {
    value_set_bool(v, widget->loading);
  }
  else if (tk_str_eq(name, WIDGET_PROP_SELF_LAYOUT))
  {
    if (widget->self_layout != NULL)
    {
      value_set_str(v, self_layouter_to_string(widget->self_layout));
    }
    else
    {
      ret = RET_NOT_FOUND;
    }
  }
  else if (tk_str_eq(name, WIDGET_PROP_CHILDREN_LAYOUT))
  {
    if (widget->children_layout != NULL)
    {
      value_set_str(v, children_layouter_to_string(widget->children_layout));
    }
    else
    {
      ret = RET_NOT_FOUND;
    }
  }
  else
  {
    if (widget->vt->get_prop)
    {
      ret = widget->vt->get_prop(widget, name, v);
    }
    else
    {
      ret = RET_NOT_FOUND;
    }
  }

  /*default*/
  if (ret == RET_NOT_FOUND)
  {
    if (tk_str_eq(name, WIDGET_PROP_LAYOUT_W))
    {
      value_set_int32(v, widget->w);
      ret = RET_OK;
    }
    else if (tk_str_eq(name, WIDGET_PROP_LAYOUT_H))
    {
      value_set_int32(v, widget->h);
      ret = RET_OK;
    }
    else if (tk_str_eq(name, WIDGET_PROP_TR_TEXT))
    {
      value_set_str(v, widget->tr_text);
      ret = RET_OK;
    }
    else if (tk_str_eq(name, WIDGET_PROP_TEXT))
    {
      wchar_t *text = widget->text.str;
      if (text != NULL)
      {
        text[widget->text.size] = 0;
      }
      value_set_wstr(v, text);
      ret = RET_OK;
    }
    else if (tk_str_eq(name, WIDGET_PROP_STATE_FOR_STYLE))
    {
      value_set_str(v, widget_get_state_for_style(widget, FALSE, FALSE));
      ret = RET_OK;
    }
  }

  if (ret == RET_NOT_FOUND)
  {
    if (widget->custom_props != NULL)
    {
      ret = tk_object_get_prop(widget->custom_props, name, v);
    }
  }

  if (ret == RET_NOT_FOUND)
  {
    if (tk_str_eq(name, WIDGET_PROP_TYPE))
    {
      value_set_str(v, widget->vt->type);
      ret = RET_OK;
    }
  }

  return ret;
}

ret_t widget_set_prop_str(widget_t *widget, const char *name, const char *str)
{
  value_t v;
  value_set_str(&v, str);

  return widget_set_prop(widget, name, &v);
}

const char *widget_get_prop_str(widget_t *widget, const char *name, const char *defval)
{
  value_t v;
  if (widget_get_prop(widget, name, &v) == RET_OK)
  {
    return value_str(&v);
  }
  else
  {
    return defval;
  }
}

ret_t widget_set_prop_pointer(widget_t *widget, const char *name, void *pointer)
{
  return widget_set_prop_pointer_ex(widget, name, pointer, NULL);
}

ret_t widget_set_prop_pointer_ex(widget_t *widget, const char *name, void *pointer,
                                 tk_destroy_t destroy)
{
  value_t v;
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && name != NULL, RET_BAD_PARAMS);

  value_set_pointer_ex(&v, pointer, destroy);
  ret = widget_set_prop(widget, name, &v);
  value_reset(&v);

  return ret;
}

void *widget_get_prop_pointer(widget_t *widget, const char *name)
{
  value_t v;
  if (widget_get_prop(widget, name, &v) == RET_OK)
  {
    return value_pointer(&v);
  }
  else
  {
    return NULL;
  }
}

ret_t widget_set_prop_float(widget_t *widget, const char *name, float_t num)
{
  value_t v;
  value_set_float32(&v, num);

  return widget_set_prop(widget, name, &v);
}

float_t widget_get_prop_float(widget_t *widget, const char *name, float_t defval)
{
  value_t v;
  if (widget_get_prop(widget, name, &v) == RET_OK)
  {
    return value_float32(&v);
  }
  else
  {
    return defval;
  }
}

ret_t widget_set_prop_int(widget_t *widget, const char *name, int32_t num)
{
  value_t v;
  value_set_int(&v, num);

  return widget_set_prop(widget, name, &v);
}

int32_t widget_get_prop_int(widget_t *widget, const char *name, int32_t defval)
{
  value_t v;
  if (widget_get_prop(widget, name, &v) == RET_OK)
  {
    return value_int(&v);
  }
  else
  {
    return defval;
  }
}

ret_t widget_set_prop_bool(widget_t *widget, const char *name, bool_t num)
{
  value_t v;
  value_set_bool(&v, num);

  return widget_set_prop(widget, name, &v);
}

bool_t widget_get_prop_bool(widget_t *widget, const char *name, bool_t defval)
{
  value_t v;
  if (widget_get_prop(widget, name, &v) == RET_OK)
  {
    return value_bool(&v);
  }
  else
  {
    return defval;
  }
}

ret_t widget_on_paint_background(widget_t *widget, canvas_t *c)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->vt->on_paint_background)
  {
    ret = widget->vt->on_paint_background(widget, c);
  }
  else
  {
    if (style_is_valid(widget->astyle))
    {
      widget_draw_background(widget, c);
    }
  }

  return ret;
}

ret_t widget_on_paint_self(widget_t *widget, canvas_t *c)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->vt->on_paint_self)
  {
    ret = widget->vt->on_paint_self(widget, c);
  }
  else
  {
    paint_event_t e;
    widget_dispatch(widget, paint_event_init(&e, EVT_PAINT, widget, c));
  }

  return ret;
}

ret_t widget_on_paint_children(widget_t *widget, canvas_t *c)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->vt->on_paint_children)
  {
    ret = widget->vt->on_paint_children(widget, c);
  }
  else
  {
    ret = widget_on_paint_children_default(widget, c);
  }

  return ret;
}

ret_t widget_on_paint_border(widget_t *widget, canvas_t *c)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->vt->on_paint_border)
  {
    ret = widget->vt->on_paint_border(widget, c);
  }
  else
  {
    if (style_is_valid(widget->astyle))
    {
      ret = widget_draw_border(widget, c);
    }
  }

  return ret;
}

static ret_t widget_on_paint_begin(widget_t *widget, canvas_t *c)
{
  paint_event_t e;
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->vt->on_paint_begin)
  {
    ret = widget->vt->on_paint_begin(widget, c);
  }

  widget_dispatch(widget, paint_event_init(&e, EVT_BEFORE_PAINT, widget, c));

  return ret;
}

static ret_t widget_on_paint_done(widget_t *widget, canvas_t *c)
{
  paint_event_t e;
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  widget_dispatch(widget, paint_event_init(&e, EVT_PAINT_DONE, widget, c));

  return ret;
}

static ret_t widget_on_paint_end(widget_t *widget, canvas_t *c)
{
  paint_event_t e;
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->vt->on_paint_end)
  {
    ret = widget->vt->on_paint_end(widget, c);
  }

  widget_dispatch(widget, paint_event_init(&e, EVT_AFTER_PAINT, widget, c));

  return ret;
}

ret_t widget_dispatch_to_target(widget_t *widget, event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  e->target = widget;
  widget_dispatch(widget, e);

  if (widget->target)
  {
    ret = widget_dispatch_to_target(widget->target, e);
  }

  return ret;
}

static ret_t widget_map_key(widget_t *widget, key_event_t *e)
{
  value_t v;
  const key_type_value_t *kv = NULL;

  if (widget->custom_props != NULL)
  {
    kv = keys_type_find_by_value(e->key);
    if (kv != NULL)
    {
      const char *to = NULL;
      char from[TK_NAME_LEN + 1] = {0};
      char fixed_name[TK_NAME_LEN + 1];

      tk_snprintf(from, sizeof(from), "map_key:%s", kv->name);
      if (tk_object_get_prop(widget->custom_props, from, &v) == RET_OK)
      {
        to = value_str(&v);
      }
      else if (strlen(kv->name) > 1)
      {
        tk_strcpy(fixed_name, kv->name);
        tk_str_tolower(fixed_name);
        tk_snprintf(from, sizeof(from), "map_key:%s", fixed_name);
        if (tk_object_get_prop(widget->custom_props, from, &v) == RET_OK)
        {
          to = value_str(&v);
        }
      }

      if (to != NULL)
      {
        kv = keys_type_find(to);
        if (kv != NULL)
        {
          e->key = kv->value;
          log_debug("map key %s to %s\n", from, to);
        }
      }
    }
  }

  return RET_OK;
}

ret_t widget_dispatch_to_key_target(widget_t *widget, event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  e->target = widget;
  widget_dispatch(widget, e);

  if (widget->key_target)
  {
    ret = widget_dispatch_to_target(widget->key_target, e);
  }

  return ret;
}

static ret_t widget_on_keydown_before_children(widget_t *widget, key_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  if (widget->emitter != NULL)
  {
    key_event_t before = *e;
    before.e.type = EVT_KEY_DOWN_BEFORE_CHILDREN;
    return_value_if_equal(emitter_dispatch(widget->emitter, (event_t *)&(before)), RET_STOP);
  }

  return widget_on_event_before_children(widget, (event_t *)e);
}

static ret_t widget_on_keydown_children(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;

  if (widget->key_target != NULL)
  {
    ret = widget_on_keydown(widget->key_target, e);
  }

  return ret;
}

static ret_t widget_on_keydown_after_children(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;

  return_value_if_equal(ret = widget_dispatch(widget, (event_t *)e), RET_STOP);
  if (widget->vt->on_keydown)
  {
    ret = widget->vt->on_keydown(widget, e);
  }

  return ret;
}

bool_t widget_is_activate_key(widget_t *widget, key_event_t *e)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL && e != NULL, FALSE);

  return (widget->vt->space_key_to_activate && e->key == TK_KEY_SPACE) ||
         (widget->vt->return_key_to_activate && key_code_is_enter(e->key));
}

static bool_t shortcut_fast_match(const char *shortcut, key_event_t *e)
{
  uint32_t key = e->key;
  const char *kname = strrchr(shortcut, '+');
  bool_t cmd = strstr(shortcut, "cmd") != NULL;
  bool_t ctrl = strstr(shortcut, "ctrl") != NULL;
  bool_t shift = strstr(shortcut, "shift") != NULL;
  const key_type_value_t *kv = keys_type_find_by_value(key);

  if (kv != NULL)
  {
    if (kname == NULL)
    {
      kname = shortcut;
    }
    else
    {
      kname++;
    }

    if (tk_str_ieq(kname, kv->name) && cmd == e->cmd && ctrl == e->ctrl && shift == e->shift)
    {
      return TRUE;
    }
  }

  return FALSE;
}

static bool_t widget_match_key(widget_t *widget, const char *prop, key_event_t *e)
{
  const char *shortcut = NULL;
  widget_t *win = widget_get_window(widget);

  if (widget_is_window_manager(widget))
  {
    return FALSE;
  }

  return_value_if_fail(win != NULL, FALSE);
  shortcut = widget_get_prop_str(win, prop, NULL);

  if (shortcut != NULL)
  {
    return shortcut_fast_match(shortcut, e);
  }

  return FALSE;
}

static bool_t widget_is_move_focus_prev_key(widget_t *widget, key_event_t *e)
{
  return widget_match_key(widget, WIDGET_PROP_MOVE_FOCUS_PREV_KEY, e);
}

static bool_t widget_is_move_focus_next_key(widget_t *widget, key_event_t *e)
{
  return widget_match_key(widget, WIDGET_PROP_MOVE_FOCUS_NEXT_KEY, e);
}

static bool_t widget_is_move_focus_up_key(widget_t *widget, key_event_t *e)
{
  return widget_match_key(widget, WIDGET_PROP_MOVE_FOCUS_UP_KEY, e);
}

static bool_t widget_is_move_focus_down_key(widget_t *widget, key_event_t *e)
{
  return widget_match_key(widget, WIDGET_PROP_MOVE_FOCUS_DOWN_KEY, e);
}

static bool_t widget_is_move_focus_left_key(widget_t *widget, key_event_t *e)
{
  return widget_match_key(widget, WIDGET_PROP_MOVE_FOCUS_LEFT_KEY, e);
}

static bool_t widget_is_move_focus_right_key(widget_t *widget, key_event_t *e)
{
  return widget_match_key(widget, WIDGET_PROP_MOVE_FOCUS_RIGHT_KEY, e);
}

bool_t widget_is_change_focus_key(widget_t *widget, key_event_t *e)
{
  return widget_is_move_focus_prev_key(widget, e) || widget_is_move_focus_next_key(widget, e) ||
         widget_is_move_focus_up_key(widget, e) || widget_is_move_focus_down_key(widget, e) ||
         widget_is_move_focus_left_key(widget, e) || widget_is_move_focus_right_key(widget, e);
}

static ret_t widget_on_keydown_general(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;
  if (!widget_is_window_manager(widget))
  {
    if (widget_is_activate_key(widget, e))
    {
      ret = RET_STOP;
      widget_set_state(widget, WIDGET_STATE_PRESSED);
    }
    else if (widget_is_move_focus_next_key(widget, e))
    {
      if (widget_is_focusable(widget))
      {
        ret = RET_STOP;
        widget_focus_next(widget);
      }
      else if (widget_is_window(widget) && !widget_has_focused_widget_in_window(widget))
      {
        ret = RET_STOP;
        widget_focus_first(widget);
      }
    }
    else if (widget_is_move_focus_prev_key(widget, e))
    {
      if (widget_is_focusable(widget))
      {
        ret = RET_STOP;
        widget_focus_prev(widget);
      }
      else if (widget_is_window(widget) && !widget_has_focused_widget_in_window(widget))
      {
        ret = RET_STOP;
        widget_focus_first(widget);
      }
    }
    else if (widget_is_move_focus_up_key(widget, e))
    {
      if (widget_is_focusable(widget))
      {
        ret = RET_STOP;
        widget_focus_up(widget);
      }
    }
    else if (widget_is_move_focus_down_key(widget, e))
    {
      if (widget_is_focusable(widget))
      {
        ret = RET_STOP;
        widget_focus_down(widget);
      }
    }
    else if (widget_is_move_focus_left_key(widget, e))
    {
      if (widget_is_focusable(widget))
      {
        ret = RET_STOP;
        widget_focus_left(widget);
      }
    }
    else if (widget_is_move_focus_right_key(widget, e))
    {
      if (widget_is_focusable(widget))
      {
        ret = RET_STOP;
        widget_focus_right(widget);
      }
    }
  }

  return ret;
}

static ret_t widget_on_keydown_impl(widget_t *widget, key_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  return_value_if_equal(widget_on_keydown_before_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_keydown_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_keydown_after_children(widget, e), RET_STOP);

  return RET_OK;
}

ret_t widget_on_keydown(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;
  uint32_t key = e->key;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  widget_map_key(widget, e);
  if (e->e.type == EVT_KEY_DOWN)
  {
    ret = widget_on_keydown_impl(widget, e);
    if (widget->feedback)
    {
      ui_feedback_request(widget, (event_t *)e);
    }

    e->key = key;
    if (ret != RET_STOP)
    {
      ret = widget_on_keydown_general(widget, e);
    }
  }
  else if (e->e.type == EVT_KEY_LONG_PRESS)
  {
    return_value_if_equal(widget_on_keydown_children(widget, e), RET_STOP);
    ret = widget_on_keydown_after_children(widget, e);
  }
  widget_unref(widget);

  return ret;
}

static ret_t widget_on_keyup_before_children(widget_t *widget, key_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  if (widget->emitter != NULL)
  {
    key_event_t before = *e;
    before.e.type = EVT_KEY_UP_BEFORE_CHILDREN;
    return_value_if_equal(emitter_dispatch(widget->emitter, (event_t *)&(before)), RET_STOP);
  }

  return widget_on_event_before_children(widget, (event_t *)e);
}

static ret_t widget_on_keyup_children(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;

  if (widget->key_target != NULL)
  {
    ret = widget_on_keyup(widget->key_target, e);
  }

  return ret;
}

static ret_t widget_on_keyup_after_children(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;

  return_value_if_equal(ret = widget_dispatch(widget, (event_t *)e), RET_STOP);
  if (widget->vt->on_keyup)
  {
    ret = widget->vt->on_keyup(widget, e);
  }

  return ret;
}

static ret_t widget_on_keyup_impl(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  return_value_if_equal(widget_on_keyup_before_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_keyup_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_keyup_after_children(widget, e), RET_STOP);

  if (widget_is_activate_key(widget, e))
  {
    pointer_event_t click;
    if (widget_is_focusable(widget))
    {
      widget_set_state(widget, WIDGET_STATE_FOCUSED);
    }
    else
    {
      widget_set_state(widget, WIDGET_STATE_NORMAL);
    }
    widget_dispatch_async(widget, pointer_event_init(&click, EVT_CLICK, widget, 0, 0));

    ret = RET_STOP;
  }
  else if (widget_is_move_focus_next_key(widget, e))
  {
    if (widget_is_focusable(widget))
    {
      ret = RET_STOP;
    }
    else if (widget_is_window(widget) && !widget_has_focused_widget_in_window(widget))
    {
      ret = RET_STOP;
    }
  }
  else if (widget_is_move_focus_prev_key(widget, e))
  {
    if (widget_is_focusable(widget))
    {
      ret = RET_STOP;
    }
    else if (widget_is_window(widget) && !widget_has_focused_widget_in_window(widget))
    {
      ret = RET_STOP;
    }
  }
  else if (widget_is_move_focus_up_key(widget, e))
  {
    if (widget_is_focusable(widget))
    {
      ret = RET_STOP;
    }
  }
  else if (widget_is_move_focus_down_key(widget, e))
  {
    if (widget_is_focusable(widget))
    {
      ret = RET_STOP;
    }
  }
  else if (widget_is_move_focus_left_key(widget, e))
  {
    if (widget_is_focusable(widget))
    {
      ret = RET_STOP;
    }
  }
  else if (widget_is_move_focus_right_key(widget, e))
  {
    if (widget_is_focusable(widget))
    {
      ret = RET_STOP;
    }
  }

  return ret;
}

ret_t widget_on_keyup(widget_t *widget, key_event_t *e)
{
  ret_t ret = RET_OK;
  uint32_t key = e->key;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  widget_map_key(widget, e);
  ret = widget_on_keyup_impl(widget, e);
  if (widget->feedback)
  {
    ui_feedback_request(widget, (event_t *)e);
  }
  widget_unref(widget);

  e->key = key;

  return ret;
}

static ret_t widget_on_wheel_before_children(widget_t *widget, wheel_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  if (widget->emitter != NULL)
  {
    wheel_event_t before = *e;
    before.e.type = EVT_WHEEL_BEFORE_CHILDREN;
    return_value_if_equal(emitter_dispatch(widget->emitter, (event_t *)&(before)), RET_STOP);
  }

  return widget_on_event_before_children(widget, (event_t *)e);
}

static ret_t widget_on_wheel_children(widget_t *widget, wheel_event_t *e)
{
  ret_t ret = RET_OK;

  if (widget->key_target != NULL)
  {
    ret = widget_on_wheel(widget->key_target, e);
  }

  return ret;
}

static ret_t widget_on_wheel_after_children(widget_t *widget, wheel_event_t *e)
{
  ret_t ret = RET_OK;

  return_value_if_equal(ret = widget_dispatch(widget, (event_t *)e), RET_STOP);
  if (widget->vt->on_wheel)
  {
    ret = widget->vt->on_wheel(widget, e);
  }

  return ret;
}

static ret_t widget_on_wheel_impl(widget_t *widget, wheel_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  return_value_if_equal(widget_on_wheel_before_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_wheel_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_wheel_after_children(widget, e), RET_STOP);

  return RET_OK;
}

ret_t widget_on_wheel(widget_t *widget, wheel_event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  ret = widget_on_wheel_impl(widget, e);
  widget_unref(widget);

  return ret;
}

ret_t widget_on_multi_gesture(widget_t *widget, multi_gesture_event_t *e)
{
  ret_t ret = RET_OK;
  widget_t *target = NULL;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  target = widget_find_target(widget, e->x, e->y);
  if (target != NULL)
  {
    ret = widget_dispatch(target, (event_t *)e);
  }

  if (ret != RET_STOP)
  {
    ret = widget_dispatch(widget, (event_t *)e);
  }
  widget_unref(widget);

  return ret;
}

static ret_t widget_dispatch_leave_event(widget_t *widget, pointer_event_t *e)
{
  widget_t *target = widget;

  while (target != NULL)
  {
    widget_t *curr = target;
    pointer_event_t leave = *e;
    leave.e.type = EVT_POINTER_LEAVE;

    widget_dispatch(target, (event_t *)(&leave));
    target = curr->target;
    curr->target = NULL;
  }

  return RET_OK;
}

static ret_t widget_dispatch_blur_event(widget_t *widget)
{
  widget_t *target = widget;
  widget_t *temp;

  while (target != NULL)
  {
    widget_ref(target);
    if (target->focused)
    {
      target->focused = FALSE;
      event_t e = event_init(EVT_BLUR, target);
      widget_dispatch(target, &e);
      widget_set_need_update_style(target);
    }

    if (target->parent && target->parent->key_target == target)
    {
      target->parent->key_target = NULL;
    }

    temp = target->key_target;
    widget_unref(target);
    target = temp;
  }

  return RET_OK;
}

ret_t widget_dispatch_event_to_target_recursive(widget_t *widget, event_t *e)
{
  widget_t *target = NULL;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  target = widget->grab_widget ? widget->grab_widget : widget->target;
  while (target != NULL)
  {
    widget_dispatch(target, e);
    target = target->target != NULL ? target->target : target->key_target;
  }

  return RET_OK;
}

static ret_t widget_on_pointer_down_before_children(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  if (widget->emitter != NULL)
  {
    pointer_event_t before = *e;
    before.e.type = EVT_POINTER_DOWN_BEFORE_CHILDREN;
    return_value_if_equal(emitter_dispatch(widget->emitter, (event_t *)&(before)), RET_STOP);
  }

  return widget_on_event_before_children(widget, (event_t *)e);
}

ret_t widget_on_pointer_down_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;
  widget_t *target = widget_find_target(widget, e->x, e->y);

  if (target != NULL && target->enable && target->sensitive)
  {
    if (!(widget_is_keyboard(target)))
    {
      if (widget_is_focusable(target) || !widget_is_strongly_focus(widget))
      {
        if (!target->focused)
        {
          widget_set_focused_internal(target, TRUE);
        }
        else
        {
          widget->key_target = target;
        }
      }
    }
  }
  else if (widget->key_target && !widget_is_strongly_focus(widget))
  {
    widget_set_focused_internal(widget->key_target, FALSE);
  }
  return_value_if_equal(ret, RET_STOP);

  if (widget->target != target)
  {
    if (widget->target != NULL)
    {
      widget_dispatch_leave_event(widget->target, e);
    }
    widget->target = target;
  }

  if (widget->target != NULL)
  {
    ret = widget_on_pointer_down(widget->target, e);
  }

  return ret;
}

static ret_t widget_on_pointer_down_after_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;

  return_value_if_equal(ret = widget_dispatch(widget, (event_t *)e), RET_STOP);
  if (widget->vt->on_pointer_down)
  {
    return_value_if_equal(ret = widget->vt->on_pointer_down(widget, e), RET_STOP);
  }

  return ret;
}

static ret_t widget_on_pointer_down_impl(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  widget->grab_widget = NULL;
  widget->grab_widget_count = 0;
  return_value_if_equal(widget_on_pointer_down_before_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_pointer_down_children(widget, e), RET_STOP);
  return_value_if_equal(widget_on_pointer_down_after_children(widget, e), RET_STOP);

  return RET_OK;
}

ret_t widget_on_pointer_down(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  ret = widget_on_pointer_down_impl(widget, e);
  if (widget->feedback)
  {
    ui_feedback_request(widget, (event_t *)e);
  }
  widget_unref(widget);

  return ret;
}

static ret_t widget_on_pointer_move_before_children(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  if (widget->emitter != NULL)
  {
    pointer_event_t before = *e;
    before.e.type = EVT_POINTER_MOVE_BEFORE_CHILDREN;
    return_value_if_equal(emitter_dispatch(widget->emitter, (event_t *)&(before)), RET_STOP);
  }

  return widget_on_event_before_children(widget, (event_t *)e);
}

ret_t widget_on_pointer_move_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;
  widget_t *target = widget_find_target(widget, e->x, e->y);

  if (target != widget->target)
  {
    if (widget->target != NULL)
    {
      widget_dispatch_leave_event(widget->target, e);
    }

    if (target != NULL)
    {
      pointer_event_t enter = *e;
      enter.e.type = EVT_POINTER_ENTER;
      ret = widget_dispatch(target, (event_t *)(&enter));
      widget_update_pointer_cursor(target);
    }
    else
    {
      widget_update_pointer_cursor(widget);
    }

    widget->target = target;
  }
  return_value_if_equal(ret, RET_STOP);

  if (widget->target != NULL)
  {
    ret = widget_on_pointer_move(widget->target, e);
  }

  return ret;
}

static ret_t widget_on_pointer_move_after_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;

  return_value_if_equal(ret = widget_dispatch(widget, (event_t *)e), RET_STOP);
  if (widget->vt->on_pointer_move)
  {
    return_value_if_equal(ret = widget->vt->on_pointer_move(widget, e), RET_STOP);
  }

  return ret;
}

static ret_t widget_on_pointer_move_impl(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  return_value_if_equal(widget_on_pointer_move_before_children(widget, e), RET_STOP);
  if (widget_on_pointer_move_children(widget, e) == RET_STOP)
  {
    if (e->pressed)
    {
      pointer_event_t abort;
      pointer_event_init(&abort, EVT_POINTER_DOWN_ABORT, widget, e->x, e->y);
      return_value_if_equal(widget_on_pointer_move_after_children(widget, &abort), RET_STOP);
    }

    return RET_STOP;
  }
  else
  {
    return widget_on_pointer_move_after_children(widget, e);
  }
}

ret_t widget_on_pointer_move(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  ret = widget_on_pointer_move_impl(widget, e);
  widget_unref(widget);

  return ret;
}

static ret_t widget_on_pointer_up_before_children(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  if (widget->emitter != NULL)
  {
    pointer_event_t before = *e;
    before.e.type = EVT_POINTER_UP_BEFORE_CHILDREN;
    return_value_if_equal(emitter_dispatch(widget->emitter, (event_t *)&(before)), RET_STOP);
  }

  return widget_on_event_before_children(widget, (event_t *)e);
}

ret_t widget_on_pointer_up_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;

  widget_t *target = widget_find_target(widget, e->x, e->y);
  if (target != NULL)
  {
    ret = widget_on_pointer_up(target, e);
  }

  return ret;
}

static ret_t widget_on_pointer_up_after_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;

  return_value_if_equal(ret = widget_dispatch(widget, (event_t *)e), RET_STOP);
  if (widget->vt->on_pointer_up)
  {
    return_value_if_equal(ret = widget->vt->on_pointer_up(widget, e), RET_STOP);
  }

  return ret;
}

static ret_t widget_on_pointer_up_impl(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  return_value_if_equal(widget_on_pointer_up_before_children(widget, e), RET_STOP);
  if (widget_on_pointer_up_children(widget, e) == RET_STOP)
  {
    if (e->pressed)
    {
      pointer_event_t abort;
      pointer_event_init(&abort, EVT_POINTER_DOWN_ABORT, widget, e->x, e->y);
      return_value_if_equal(widget_on_pointer_up_after_children(widget, &abort), RET_STOP);
    }

    return RET_STOP;
  }
  else
  {
    return widget_on_pointer_up_after_children(widget, e);
  }
}

ret_t widget_on_pointer_up(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  ret = widget_on_pointer_up_impl(widget, e);
  if (widget->feedback)
  {
    ui_feedback_request(widget, (event_t *)e);
  }
  widget_unref(widget);

  return ret;
}

static ret_t widget_on_context_menu_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;

  widget_t *target = widget_find_target(widget, e->x, e->y);
  if (target != NULL)
  {
    ret = widget_on_context_menu(target, e);
  }

  return ret;
}

static ret_t widget_on_context_menu_after_children(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;

  return_value_if_equal(ret = widget_dispatch(widget, (event_t *)e), RET_STOP);
  if (widget->vt->on_context_menu)
  {
    return_value_if_equal(ret = widget->vt->on_context_menu(widget, e), RET_STOP);
  }

  return ret;
}

static ret_t widget_on_context_menu_impl(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (widget_on_context_menu_children(widget, e) == RET_STOP)
  {
    return RET_STOP;
  }
  else
  {
    return widget_on_context_menu_after_children(widget, e);
  }
}

ret_t widget_on_context_menu(widget_t *widget, pointer_event_t *e)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  widget_ref(widget);
  ret = widget_on_context_menu_impl(widget, e);
  widget_unref(widget);

  return ret;
}

ret_t widget_grab(widget_t *widget, widget_t *child)
{
  return_value_if_fail(widget != NULL && child != NULL && widget->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->grab_widget == NULL || widget->grab_widget == child, RET_BAD_PARAMS);

  if (widget->grab_widget == NULL)
  {
    widget->grab_widget = child;
    widget->grab_widget_count = 1;
  }
  else
  {
    widget->grab_widget_count++;
  }

  if (widget->parent)
  {
    widget_grab(widget->parent, widget);
  }

  return RET_OK;
}

ret_t widget_ungrab(widget_t *widget, widget_t *child)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->grab_widget == child)
  {
    if (widget->grab_widget->grab_widget_count < widget->grab_widget_count)
    {
      widget->grab_widget_count--;
      if (widget->grab_widget_count <= 0)
      {
        widget->grab_widget = NULL;
        widget->grab_widget_count = 0;
      }

      if (widget->parent)
      {
        widget_ungrab(widget->parent, widget);
      }
    }
  }

  return RET_OK;
}

ret_t widget_foreach(widget_t *widget, tk_visit_t visit, void *ctx)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && visit != NULL, RET_BAD_PARAMS);

  ret = visit(ctx, widget);
  if (ret != RET_OK)
  {
    return ret;
  }

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  ret = widget_foreach(iter, visit, ctx);
  if (ret == RET_STOP || ret == RET_DONE)
  {
    return ret;
  }
  WIDGET_FOR_EACH_CHILD_END()

  return RET_OK;
}

widget_t *widget_get_window(widget_t *widget)
{
  widget_t *iter = widget;
  return_value_if_fail(widget != NULL, NULL);

  while (iter)
  {
    if (widget_is_window(iter))
    {
      return iter;
    }
    iter = iter->parent;
  }

  return NULL;
}

static widget_t *widget_get_window_or_keyboard(widget_t *widget)
{
  widget_t *iter = widget;
  return_value_if_fail(widget != NULL, NULL);

  while (iter)
  {
    if (widget_is_window(iter) || widget_is_keyboard(iter))
    {
      return iter;
    }
    iter = iter->parent;
  }

  return NULL;
}

widget_t *widget_get_window_manager(widget_t *widget)
{
  widget_t *iter = widget;
  return_value_if_fail(widget != NULL, NULL);

  while (iter)
  {
    if (widget_is_window_manager(iter))
    {
      return iter;
    }
    iter = iter->parent;
  }

  return window_manager();
}

uint32_t widget_add_timer(widget_t *widget, timer_func_t on_timer, uint32_t duration_ms)
{
  return_value_if_fail(widget != NULL && on_timer != NULL, TK_INVALID_ID);
  return timer_add_with_type(on_timer, widget, duration_ms, TIMER_INFO_WIDGET_ADD);
}

ret_t widget_remove_timer(widget_t *widget, uint32_t timer_id)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);
  return timer_remove(timer_id);
}

uint32_t widget_add_idle(widget_t *widget, idle_func_t on_idle)
{
  return_value_if_fail(widget != NULL && on_idle != NULL, TK_INVALID_ID);
  return idle_add_with_type(on_idle, widget, IDLE_INFO_WIDGET_ADD);
}

ret_t widget_remove_idle(widget_t *widget, uint32_t idle_id)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);
  return idle_remove(idle_id);
}

ret_t widget_destroy_sync(widget_t *widget)
{
  event_t e = event_init(EVT_DESTROY, widget);
  return_value_if_fail(widget != NULL && widget->vt != NULL, RET_BAD_PARAMS);

#ifndef WITHOUT_WIDGET_ANIMATORS
  widget_destroy_animator(widget, NULL);
#endif /*WITHOUT_WIDGET_ANIMATORS*/

  widget->destroying = TRUE;
  idle_remove_all_by_ctx_and_type(IDLE_INFO_WIDGET_ADD, widget);
  timer_remove_all_by_ctx_and_type(TIMER_INFO_WIDGET_ADD, widget);

  if (widget->emitter != NULL)
  {
    widget_dispatch(widget, &e);
    emitter_destroy(widget->emitter);
    widget->emitter = NULL;
  }

  if (widget->children != NULL)
  {
    widget_destroy_children(widget);
    darray_destroy(widget->children);
    widget->children = NULL;
  }

  if (widget->children_layout != NULL)
  {
    children_layouter_destroy(widget->children_layout);
    widget->children_layout = NULL;
  }

  if (widget->self_layout != NULL)
  {
    self_layouter_destroy(widget->self_layout);
    widget->self_layout = NULL;
  }

  widget->destroying = FALSE;

  return widget_real_destroy(widget);
}

widget_t *widget_create(widget_t *parent, const widget_vtable_t *vt, xy_t x, xy_t y, wh_t w,
                        wh_t h)
{
  return_value_if_fail(vt != NULL, NULL);

  return widget_init(widget_real_create(vt), parent, vt, x, y, w, h);
}

ret_t widget_destroy(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->ref_count > 0 && widget->vt != NULL,
                       RET_BAD_PARAMS);

  if (widget->parent != NULL)
  {
    widget_remove_child(widget->parent, widget);
  }

  return widget_unref_async(widget);
}

static ret_t widget_destroy_on_idle(const idle_info_t *info)
{
  widget_destroy(WIDGET(info->ctx));

  return RET_REMOVE;
}

ret_t widget_destroy_async(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->ref_count > 0 && widget->vt != NULL,
                       RET_BAD_PARAMS);

  return_value_if_fail(idle_add(widget_destroy_on_idle, widget) != TK_INVALID_ID, RET_FAIL);

  return RET_OK;
}

static ret_t widget_set_parent_not_dirty(widget_t *widget)
{
  widget_t *iter = widget->parent;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  while (iter != NULL)
  {
    iter->dirty = FALSE;
    if (iter->vt->is_window)
    {
      break;
    }
    iter = iter->parent;
  }

  return RET_OK;
}

ret_t widget_invalidate(widget_t *widget, const rect_t *r)
{
  rect_t rself;
  return_value_if_fail(widget != NULL && widget->vt != NULL, RET_BAD_PARAMS);

  if (widget->dirty)
  {
    return RET_OK;
  }

  if (r == NULL)
  {
    rself = rect_init(0, 0, widget->w, widget->h);
    r = &rself;
  }

  widget->dirty = TRUE;
  widget_set_parent_not_dirty(widget);

  if (widget->vt && widget->vt->invalidate)
  {
    return widget->vt->invalidate(widget, r);
  }
  else
  {
    return widget_invalidate_default(widget, r);
  }
}

ret_t widget_invalidate_force(widget_t *widget, const rect_t *r)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  widget->dirty = FALSE;
  return widget_invalidate(widget, r);
}

widget_t *widget_init(widget_t *widget, widget_t *parent, const widget_vtable_t *vt, xy_t x, xy_t y,
                      wh_t w, wh_t h)
{
  return_value_if_fail(widget != NULL && vt != NULL, NULL);

  widget->x = x;
  widget->y = y;
  widget->w = w;
  widget->h = h;
  widget->vt = vt;
  widget->dirty = TRUE;
  widget->ref_count = 1;
  widget->opacity = 0xff;
  widget->enable = TRUE;
  widget->visible = TRUE;
  widget->feedback = FALSE;
  widget->auto_adjust_size = FALSE;
  widget->sensitive = TRUE;
  widget->emitter = NULL;
  widget->children = NULL;
  widget->initializing = TRUE;
  widget->state = tk_strdup(WIDGET_STATE_NORMAL);
  widget->target = NULL;
  widget->key_target = NULL;
  widget->grab_widget = NULL;
  widget->grab_widget_count = 0;
  widget->focused = FALSE;
  widget->focusable = FALSE;
  widget->with_focus_state = FALSE;
  widget->dirty_rect_tolerance = 4;
  widget->need_update_style = TRUE;

  if (parent)
  {
    widget_add_child(parent, widget);
  }

  wstr_init(&(widget->text), 0);
  if (!widget->vt)
  {
    widget->vt = TK_GET_VTABLE(widget);
  }

  if (widget->astyle == NULL &&
      (widget_is_window_manager(widget) || widget_get_window(widget) != NULL))
  {
    widget->astyle = style_factory_create_style(style_factory(), widget_get_style_type(widget));
    ENSURE(widget->astyle != NULL);
    if (widget_is_window_opened(widget))
    {
      widget_set_need_update_style(widget);
    }
  }

  widget_invalidate_force(widget, NULL);

  widget->initializing = FALSE;

  return widget;
}

ret_t widget_get_prop_default_value(widget_t *widget, const char *name, value_t *v)
{
  ret_t ret = RET_OK;
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_X))
  {
    value_set_int32(v, 0);
  }
  else if (tk_str_eq(name, WIDGET_PROP_Y))
  {
    value_set_int32(v, 0);
  }
  else if (tk_str_eq(name, WIDGET_PROP_W))
  {
    value_set_int32(v, 0);
  }
  else if (tk_str_eq(name, WIDGET_PROP_H))
  {
    value_set_int32(v, 0);
  }
  else if (tk_str_eq(name, WIDGET_PROP_OPACITY))
  {
    value_set_int32(v, 0xff);
  }
  else if (tk_str_eq(name, WIDGET_PROP_VISIBLE))
  {
    value_set_bool(v, TRUE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_SENSITIVE))
  {
    value_set_bool(v, TRUE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FLOATING))
  {
    value_set_bool(v, FALSE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FOCUSABLE))
  {
    value_set_bool(v, FALSE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_WITH_FOCUS_STATE))
  {
    value_set_bool(v, FALSE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_DIRTY_RECT_TOLERANCE))
  {
    value_set_int(v, 4);
  }
  else if (tk_str_eq(name, WIDGET_PROP_STYLE))
  {
    value_set_str(v, NULL);
  }
  else if (tk_str_eq(name, WIDGET_PROP_ENABLE))
  {
    value_set_bool(v, TRUE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_NAME))
  {
    value_set_str(v, NULL);
  }
  else if (tk_str_eq(name, WIDGET_PROP_TEXT))
  {
    value_set_wstr(v, NULL);
  }
  else if (tk_str_eq(name, WIDGET_PROP_ANIMATION))
  {
    value_set_str(v, NULL);
  }
  else if (tk_str_eq(name, WIDGET_PROP_SELF_LAYOUT))
  {
    value_set_str(v, NULL);
  }
  else if (tk_str_eq(name, WIDGET_PROP_CHILDREN_LAYOUT))
  {
    value_set_str(v, NULL);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FOCUSED))
  {
    value_set_bool(v, FALSE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_FEEDBACK))
  {
    value_set_bool(v, FALSE);
  }
  else if (tk_str_eq(name, WIDGET_PROP_AUTO_ADJUST_SIZE))
  {
    value_set_bool(v, FALSE);
  }
  else
  {
    if (widget->vt->get_prop_default_value)
    {
      ret = widget->vt->get_prop_default_value(widget, name, v);
    }
    else
    {
      ret = RET_NOT_FOUND;
    }
  }

  return ret;
}

ret_t widget_get_offset(widget_t *widget, xy_t *out_x, xy_t *out_y)
{
  return_value_if_fail(widget != NULL && out_x != NULL && out_y != NULL, RET_BAD_PARAMS);
  *out_x = 0;
  *out_y = 0;
  if (widget->vt != NULL && widget->vt->get_offset != NULL)
  {
    return widget->vt->get_offset(widget, out_x, out_y);
  }
  else if (widget->vt != NULL && widget->vt->get_prop != NULL)
  {
    value_t v;
    if (widget->vt->get_prop(widget, WIDGET_PROP_XOFFSET, &v) == RET_OK)
    {
      *out_x = value_int(&v);
    }
    if (widget->vt->get_prop(widget, WIDGET_PROP_YOFFSET, &v) == RET_OK)
    {
      *out_y = value_int(&v);
    }
  }
  return RET_OK;
}

ret_t widget_to_screen_ex(widget_t *widget, widget_t *parent, point_t *p)
{
  widget_t *iter = widget;
  return_value_if_fail(widget != NULL && p != NULL, RET_BAD_PARAMS);

  while (iter != NULL && iter != parent)
  {
    xy_t offset_x, offset_y;
    if (widget_get_offset(iter, &offset_x, &offset_y) == RET_OK)
    {
      p->x -= offset_x;
      p->y -= offset_y;
    }

    p->x += iter->x;
    p->y += iter->y;

    iter = iter->parent;
  }

  return RET_OK;
}

ret_t widget_to_screen(widget_t *widget, point_t *p)
{
  return widget_to_screen_ex(widget, NULL, p);
}

ret_t widget_to_local(widget_t *widget, point_t *p)
{
  widget_t *iter = widget;
  return_value_if_fail(widget != NULL && p != NULL, RET_BAD_PARAMS);

  while (iter != NULL)
  {
    xy_t offset_x = 0;
    xy_t offset_y = 0;
    if (widget_get_offset(iter, &offset_x, &offset_y) == RET_OK)
    {
      p->x += offset_x;
      p->y += offset_y;
    }

    p->x -= iter->x;
    p->y -= iter->y;

    iter = iter->parent;
  }

  return RET_OK;
}

ret_t widget_to_global(widget_t *widget, point_t *p)
{
  widget_t *iter = widget;
  return_value_if_fail(widget != NULL && p != NULL, RET_BAD_PARAMS);

  while (iter != NULL)
  {
    p->x += iter->x;
    p->y += iter->y;

    iter = iter->parent;
  }

  return RET_OK;
}

int32_t widget_count_children(widget_t *widget)
{
  return_value_if_fail(widget != NULL, 0);

  return widget->children != NULL ? widget->children->size : 0;
}

widget_t *widget_get_child(widget_t *widget, int32_t index)
{
  return_value_if_fail(widget != NULL, NULL);
  if (widget->children == NULL || index >= widget->children->size)
  {
    return NULL;
  }

  return WIDGET(widget->children->elms[index]);
}

int32_t widget_index_of(widget_t *widget)
{
  widget_t *parent = NULL;
  return_value_if_fail(widget != NULL && widget->parent != NULL, -1);

  parent = widget->parent;
  WIDGET_FOR_EACH_CHILD_BEGIN(parent, iter, i)
  if (iter == widget)
  {
    return i;
  }
  WIDGET_FOR_EACH_CHILD_END();

  return -1;
}

ret_t widget_prepare_text_style_ex(widget_t *widget, canvas_t *c, color_t default_trans,
                                   const char *default_font, uint16_t default_font_size,
                                   align_h_t default_align_h, align_v_t default_align_v)
{
  style_t *style = widget->astyle;
  color_t tc = style_get_color(style, STYLE_ID_TEXT_COLOR, default_trans);
  const char *font_name = style_get_str(style, STYLE_ID_FONT_NAME, default_font);
  uint16_t font_size = style_get_int(style, STYLE_ID_FONT_SIZE, default_font_size);
  align_h_t align_h = (align_h_t)style_get_int(style, STYLE_ID_TEXT_ALIGN_H, default_align_h);
  align_v_t align_v = (align_v_t)style_get_int(style, STYLE_ID_TEXT_ALIGN_V, default_align_v);

  canvas_set_text_color(c, tc);
  canvas_set_font(c, font_name, font_size);
  canvas_set_text_align(c, align_h, align_v);

  return RET_OK;
}

ret_t widget_prepare_text_style(widget_t *widget, canvas_t *c)
{
  color_t trans = color_init(0, 0, 0, 0);
  return widget_prepare_text_style_ex(widget, c, trans, NULL, TK_DEFAULT_FONT_SIZE, ALIGN_H_CENTER,
                                      ALIGN_V_MIDDLE);
}

static ret_t widget_copy_style(widget_t *clone, widget_t *widget)
{
  if (style_is_mutable(widget->astyle) && style_mutable_cast(widget->astyle) != NULL)
  {
    if (!style_is_mutable(clone->astyle))
    {
      widget_ensure_style_mutable(clone);
    }

    if (style_mutable_cast(clone->astyle) != NULL)
    {
      style_mutable_copy(clone->astyle, widget->astyle);
    }
  }

  return RET_OK;
}

static const char *const s_widget_persistent_props[] = {WIDGET_PROP_NAME,
                                                        WIDGET_PROP_STYLE,
                                                        WIDGET_PROP_TR_TEXT,
                                                        WIDGET_PROP_TEXT,
                                                        WIDGET_PROP_ANIMATION,
                                                        WIDGET_PROP_ENABLE,
                                                        WIDGET_PROP_VISIBLE,
                                                        WIDGET_PROP_FLOATING,
                                                        WIDGET_PROP_CHILDREN_LAYOUT,
                                                        WIDGET_PROP_SELF_LAYOUT,
                                                        WIDGET_PROP_OPACITY,
                                                        WIDGET_PROP_FOCUSED,
                                                        WIDGET_PROP_FEEDBACK,
                                                        WIDGET_PROP_AUTO_ADJUST_SIZE,
                                                        WIDGET_PROP_FOCUSABLE,
                                                        WIDGET_PROP_SENSITIVE,
                                                        WIDGET_PROP_WITH_FOCUS_STATE,
                                                        NULL};

const char *const *widget_get_persistent_props(void)
{
  return s_widget_persistent_props;
}

static ret_t widget_copy_base_props(widget_t *widget, widget_t *other)
{
  widget->state = tk_str_copy(widget->state, other->state);
  widget->name = tk_str_copy(widget->name, other->name);
  widget->style = tk_str_copy(widget->style, other->style);

  if (other->text.str != NULL)
  {
    widget_set_text(widget, other->text.str);
  }

  if (other->tr_text != NULL)
  {
    widget_set_tr_text(widget, other->tr_text);
  }

  widget->enable = other->enable;
  widget->visible = other->visible;
  widget->floating = other->floating;
  widget->opacity = other->opacity;
  widget->feedback = other->feedback;
  widget->auto_adjust_size = other->auto_adjust_size;
  widget->focusable = other->focusable;
  widget->sensitive = other->sensitive;
  widget->auto_created = other->auto_created;
  widget->with_focus_state = other->with_focus_state;
  widget->dirty_rect_tolerance = other->dirty_rect_tolerance;

  if (other->animation != NULL && *(other->animation))
  {
    widget_set_animation(widget, other->animation);
  }

  if (other->self_layout != NULL)
  {
    widget->self_layout = self_layouter_clone(other->self_layout);
  }

  if (other->children_layout != NULL)
  {
    widget->children_layout = children_layouter_clone(other->children_layout);
  }

  return RET_OK;
}

static ret_t widget_copy(widget_t *widget, widget_t *other)
{
  return_value_if_fail(widget != NULL && other != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget->vt == other->vt, RET_BAD_PARAMS);

  widget_copy_style(widget, other);
  widget_copy_base_props(widget, other);

  if (other->custom_props)
  {
    widget->custom_props = object_default_clone(OBJECT_DEFAULT(other->custom_props));
  }

  if (widget->vt->on_copy != NULL)
  {
    widget->vt->on_copy(widget, other);
  }
  else
  {
    widget_on_copy_default(widget, other);
  }

  widget_set_need_update_style(widget);

  return RET_OK;
}

widget_t *widget_clone(widget_t *widget, widget_t *parent)
{
  widget_t *clone = NULL;
  return_value_if_fail(widget != NULL && widget->vt != NULL && widget->vt->create != NULL, NULL);

  clone = widget->vt->create(parent, widget->x, widget->y, widget->w, widget->h);
  return_value_if_fail(clone != NULL, NULL);

  widget_copy(clone, widget);

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  widget_clone(iter, clone);
  WIDGET_FOR_EACH_CHILD_END();

  return clone;
}

#define PROP_EQ(prop) (widget->prop == other->prop)
bool_t widget_equal(widget_t *widget, widget_t *other)
{
  bool_t ret = FALSE;
  const char *const *properties = NULL;
  return_value_if_fail(widget != NULL && other != NULL, FALSE);

  ret = PROP_EQ(opacity) && PROP_EQ(enable) && PROP_EQ(visible) && PROP_EQ(vt) && PROP_EQ(x) &&
        PROP_EQ(y) && PROP_EQ(w) && PROP_EQ(h) && PROP_EQ(floating);
  if (widget->name != NULL || other->name != NULL)
  {
    ret = ret && (tk_str_eq(widget->name, other->name) || PROP_EQ(name));
  }

  if (widget->style != NULL || other->style != NULL)
  {
    ret = ret && tk_str_eq(widget->style, other->style);
  }

  if (!ret)
  {
    return ret;
  }

  ret = ret && wstr_equal(&(widget->text), &(other->text));

  if (widget->tr_text != NULL || other->tr_text != NULL)
  {
    ret = ret && (tk_str_eq(widget->tr_text, other->tr_text) || PROP_EQ(tr_text));
  }

  if (!ret)
  {
    return ret;
  }

  properties = widget->vt->clone_properties;
  if (properties != NULL)
  {
    value_t v1;
    value_t v2;
    uint32_t i = 0;
    for (i = 0; properties[i] != NULL; i++)
    {
      const char *prop = properties[i];
      if (widget_get_prop(widget, prop, &v1) != RET_OK)
      {
        continue;
      }

      if (widget_get_prop(other, prop, &v2) != RET_OK)
      {
        return FALSE;
      }

      if (!value_equal(&v1, &v2))
      {
        log_debug("prop %s not equal\n", prop);
        return FALSE;
      }
    }
  }

  if (widget->children_layout != other->children_layout)
  {
    const char *str1 = children_layouter_to_string(widget->children_layout);
    const char *str2 = children_layouter_to_string(other->children_layout);
    if (!tk_str_eq(str1, str2))
    {
      return FALSE;
    }
  }

  if (widget->self_layout != other->self_layout)
  {
    const char *str1 = self_layouter_to_string(widget->self_layout);
    const char *str2 = self_layouter_to_string(other->self_layout);
    if (!tk_str_eq(str1, str2))
    {
      return FALSE;
    }
  }

  if (!ret)
  {
    return ret;
  }

  if (widget->children == other->children)
  {
    return TRUE;
  }

  if (widget->children == NULL || other->children == NULL)
  {
    return FALSE;
  }

  if (widget->children->size != other->children->size)
  {
    return FALSE;
  }

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  widget_t *iter_other = WIDGET(other->children->elms[i]);
  if (!widget_equal(iter, iter_other))
  {
    return FALSE;
  }
  WIDGET_FOR_EACH_CHILD_END();

  return TRUE;
}

float_t widget_measure_text(widget_t *widget, const wchar_t *text)
{
  canvas_t *c = widget_get_canvas(widget);
  return_value_if_fail(widget != NULL && text != NULL && c != NULL, 0);

  widget_prepare_text_style(widget, c);

  return canvas_measure_text(c, (wchar_t *)text, wcslen(text));
}

ret_t widget_load_image(widget_t *widget, const char *name, bitmap_t *bitmap)
{
  image_manager_t *imm = widget_get_image_manager(widget);

  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget != NULL && name != NULL && bitmap != NULL, RET_BAD_PARAMS);

  return image_manager_get_bitmap(imm, name, bitmap);
}

ret_t widget_unload_image(widget_t *widget, bitmap_t *bitmap)
{
  image_manager_t *imm = widget_get_image_manager(widget);

  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget != NULL && bitmap != NULL, RET_BAD_PARAMS);

  return image_manager_unload_bitmap(imm, bitmap);
}

const asset_info_t *widget_load_asset(widget_t *widget, asset_type_t type, const char *name)
{
  return widget_load_asset_ex(widget, type, 0, name);
}

const asset_info_t *widget_load_asset_ex(widget_t *widget, asset_type_t type, uint16_t subtype,
                                         const char *name)
{
  assets_manager_t *am = widget_get_assets_manager(widget);
  return_value_if_fail(widget != NULL && name != NULL && am != NULL, NULL);

  return assets_manager_ref_ex(am, type, subtype, name);
}

ret_t widget_unload_asset(widget_t *widget, const asset_info_t *asset)
{
  assets_manager_t *am = widget_get_assets_manager(widget);
  return_value_if_fail(widget != NULL && asset != NULL && am != NULL, RET_BAD_PARAMS);

  return assets_manager_unref(am, asset);
}

bool_t widget_is_point_in(widget_t *widget, xy_t x, xy_t y, bool_t is_local)
{
  point_t p = {x, y};
  return_value_if_fail(widget != NULL, FALSE);

  if (!is_local)
  {
    widget_to_local(widget, &p);
  }

  if (widget->vt->is_point_in != NULL)
  {
    return widget->vt->is_point_in(widget, p.x, p.y);
  }
  else
  {
    return (p.x >= 0 && p.y >= 0 && p.x < widget->w && p.y < widget->h);
  }
}

const char *widget_get_type(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, NULL);

  return widget_get_prop_str(widget, WIDGET_PROP_TYPE, NULL);
}

widget_t *widget_cast(widget_t *widget)
{
  return widget;
}

bool_t widget_is_instance_of(widget_t *widget, const widget_vtable_t *vt)
{
  const widget_vtable_t *iter = NULL;
  return_value_if_fail(widget != NULL && vt != NULL, FALSE);

  iter = widget->vt;
  while (iter != NULL)
  {
    if (iter == vt)
    {
      return TRUE;
    }

    iter = widget_get_parent_vtable(iter);
  }
  if (vt == TK_GET_VTABLE(widget))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static ret_t widget_ensure_visible_in_scroll_view(widget_t *scroll_view, widget_t *widget)
{
  rect_t r;
  point_t p;
  int32_t ox = 0;
  int32_t oy = 0;
  int32_t old_ox = 0;
  int32_t old_oy = 0;
  return_value_if_fail(widget != NULL && scroll_view != NULL, RET_BAD_PARAMS);

  memset(&p, 0x0, sizeof(point_t));
  widget_to_screen_ex(widget, scroll_view, &p);
  r = rect_init(p.x, p.y, widget->w, widget->h);

  ox = widget_get_prop_int(scroll_view, WIDGET_PROP_XOFFSET, 0);
  oy = widget_get_prop_int(scroll_view, WIDGET_PROP_YOFFSET, 0);
  old_ox = ox;
  old_oy = oy;

  if (oy > r.y)
  {
    oy = r.y;
  }

  if (ox > r.x)
  {
    ox = r.x;
  }

  if ((r.y + r.h) > (oy + scroll_view->h))
  {
    oy = r.y + r.h - scroll_view->h;
  }

  if ((r.x + r.w) > (ox + scroll_view->w))
  {
    ox = r.x + r.w - scroll_view->w;
  }

  if (ox != old_ox)
  {
    widget_set_prop_int(scroll_view, WIDGET_PROP_XOFFSET, ox);
  }

  if (oy != old_oy)
  {
    widget_set_prop_int(scroll_view, WIDGET_PROP_YOFFSET, oy);
  }

  return RET_OK;
}

ret_t widget_ensure_visible_in_viewport(widget_t *widget)
{
  widget_t *parent = NULL;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  parent = widget->parent;
  while (parent != NULL)
  {
    if (widget_is_scrollable(parent))
    {
      widget_ensure_visible_in_scroll_view(parent, widget);
      break;
    }

    parent = parent->parent;
  }

  return RET_OK;
}

ret_t widget_set_as_key_target(widget_t *widget)
{
  if (widget_is_keyboard(widget))
  {
    return RET_OK;
  }

  if (widget != NULL)
  {
    widget_t *parent = widget->parent;

    if (parent != NULL)
    {
      if (!(parent->focused))
      {
        parent->focused = TRUE;
        event_t e = event_init(EVT_FOCUS, NULL);
        widget_dispatch(parent, &e);
        widget_set_need_update_style(parent);
      }

      if (parent->key_target != NULL && parent->key_target != widget)
      {
        widget_set_focused_internal(widget->parent->key_target, FALSE);
      }

      if (parent->key_target != widget)
      {
        parent->key_target = widget;
      }
      widget_set_as_key_target(parent);
      if (!widget->focused)
      {
        widget_set_need_update_style(widget);
      }
    }
  }

  return RET_OK;
}

bool_t widget_is_keyboard(widget_t *widget)
{
  value_t v;
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  if (widget->vt->is_keyboard)
  {
    return TRUE;
  }

  if (widget_get_prop(widget, WIDGET_PROP_IS_KEYBOARD, &v) == RET_OK)
  {
    return value_bool(&v);
  }

  return FALSE;
}

ret_t widget_on_visit_focusable(void *ctx, const void *data)
{
  widget_t *widget = WIDGET(data);
  darray_t *all_focusable = (darray_t *)ctx;

  if (!(widget->sensitive) || !(widget->visible) || !(widget->enable))
  {
    return RET_SKIP;
  }

  if (widget->vt->get_only_active_children != NULL)
  {
    return widget->vt->get_only_active_children(widget, all_focusable);
  }

  if (widget_is_focusable(widget))
  {
    darray_push(all_focusable, widget);
  }

  if (widget->vt->disallow_children_focusable)
  {
    return RET_SKIP;
  }
  return RET_OK;
}

static ret_t widget_get_all_focusable_widgets_in_window(widget_t *widget, darray_t *all_focusable)
{
  widget_t *win = widget_get_window_or_keyboard(widget);
  return_value_if_fail(win != NULL, RET_BAD_PARAMS);

  widget_foreach(win, widget_on_visit_focusable, all_focusable);

  return RET_OK;
}

static widget_t *widget_get_first_focusable_widget_in_window(widget_t *widget)
{
  widget_t *first = NULL;
  darray_t all_focusable;
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(win != NULL, first);

  darray_init(&all_focusable, 10, NULL, NULL);
  widget_foreach(win, widget_on_visit_focusable, &all_focusable);
  if (all_focusable.size > 0)
  {
    first = WIDGET(all_focusable.elms[0]);
  }
  darray_deinit(&all_focusable);

  return first;
}

bool_t widget_has_focused_widget_in_window(widget_t *widget)
{
  widget_t *iter = NULL;
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(win != NULL, FALSE);

  iter = win->key_target;
  while (iter != NULL)
  {
    if (widget_is_focusable(iter) && iter->focused)
    {
      return TRUE;
    }

    iter = iter->key_target;
  }

  return FALSE;
}

ret_t widget_focus_first(widget_t *widget)
{
  widget_t *first = widget_get_first_focusable_widget_in_window(widget);

  if (first != NULL)
  {
    widget_set_prop_bool(first, WIDGET_PROP_FOCUSED, TRUE);
  }

  return RET_OK;
}

static widget_t *widget_find_prev_focus_widget(widget_t *widget, darray_t *all_focusable)
{
  uint32_t i = 0;
  for (i = 0; i < all_focusable->size; i++)
  {
    widget_t *iter = WIDGET(all_focusable->elms[i]);

    if (iter == widget)
    {
      uint32_t focus = (i == 0) ? (all_focusable->size - 1) : (i - 1);

      return WIDGET(all_focusable->elms[focus]);
    }
  }

  return NULL;
}

static widget_t *widget_find_next_focus_widget(widget_t *widget, darray_t *all_focusable)
{
  uint32_t i = 0;
  for (i = 0; i < all_focusable->size; i++)
  {
    widget_t *iter = WIDGET(all_focusable->elms[i]);

    if (iter == widget)
    {
      uint32_t focus = ((i + 1) == all_focusable->size) ? (0) : (i + 1);

      return WIDGET(all_focusable->elms[focus]);
    }
  }

  return NULL;
}

static bool_t is_same_row(const rect_t *r1, const rect_t *r2)
{
  int32_t cy1 = r1->y + r1->h / 2;
  int32_t cy2 = r2->y + r2->h / 2;

  return (cy2 >= r1->y && cy2 < (r1->y + r1->h)) || (cy1 >= r2->y && cy1 < (r2->y + r2->h));
}

static bool_t is_same_col(const rect_t *r1, const rect_t *r2)
{
  int32_t cx1 = r1->x + r1->w / 2;
  int32_t cx2 = r2->x + r2->w / 2;

  return (cx2 >= r1->x && cx2 < (r1->x + r1->w)) || (cx1 >= r2->x && cx1 < (r2->x + r2->w));
}

static uint32_t distance2(const rect_t *r1, const rect_t *r2)
{
  uint32_t dx = (r1->x + r1->w / 2) - (r2->x + r2->w / 2);
  uint32_t dy = (r1->y + r1->h / 2) - (r2->y + r2->h / 2);

  return dx * dx + dy * dy;
}

static bool_t match_up(const rect_t *widget, const rect_t *last_matched, const rect_t *iter)
{
  if ((iter->y + iter->h / 2) > widget->y)
  {
    return FALSE;
  }

  if (last_matched == NULL)
  {
    return TRUE;
  }

  if (is_same_col(widget, iter))
  {
    if (!is_same_col(widget, last_matched))
    {
      return TRUE;
    }
    else
    {
      return (iter->y + iter->h) > (last_matched->y + last_matched->h);
    }
  }
  else if (is_same_col(widget, last_matched))
  {
    return FALSE;
  }
  else
  {
    return distance2(widget, iter) < distance2(widget, last_matched);
  }
}

static bool_t match_down(const rect_t *widget, const rect_t *last_matched, const rect_t *iter)
{
  if ((iter->y + iter->h / 2) < (widget->y + widget->h))
  {
    return FALSE;
  }

  if (last_matched == NULL)
  {
    return TRUE;
  }

  if (is_same_col(widget, iter))
  {
    if (!is_same_col(widget, last_matched))
    {
      return TRUE;
    }
    else
    {
      return iter->y < last_matched->y;
    }
  }
  else if (is_same_col(widget, last_matched))
  {
    return FALSE;
  }
  else
  {
    return distance2(widget, iter) < distance2(widget, last_matched);
  }
}

static bool_t match_left(const rect_t *widget, const rect_t *last_matched, const rect_t *iter)
{
  if ((iter->x + iter->w / 2) > widget->x)
  {
    return FALSE;
  }

  if (last_matched == NULL)
  {
    return TRUE;
  }

  if (is_same_row(widget, iter))
  {
    if (!is_same_row(widget, last_matched))
    {
      return TRUE;
    }
    else
    {
      return (iter->x + iter->w) > (last_matched->x + last_matched->w);
    }
  }
  else if (is_same_row(widget, last_matched))
  {
    return FALSE;
  }
  else
  {
    return distance2(widget, iter) < distance2(widget, last_matched);
  }
}

static bool_t match_right(const rect_t *widget, const rect_t *last_matched, const rect_t *iter)
{
  if ((iter->x + iter->w / 2) < (widget->x + widget->w))
  {
    return FALSE;
  }

  if (last_matched == NULL)
  {
    return TRUE;
  }

  if (is_same_row(widget, iter))
  {
    if (!is_same_row(widget, last_matched))
    {
      return TRUE;
    }
    else
    {
      return iter->x < last_matched->x;
    }
  }
  else if (is_same_row(widget, last_matched))
  {
    return FALSE;
  }
  else
  {
    return distance2(widget, iter) < distance2(widget, last_matched);
  }
}

typedef bool_t (*match_focus_widget_t)(const rect_t *widget, const rect_t *last_matched,
                                       const rect_t *iter);

static widget_t *widget_find_matched_focus_widget(widget_t *widget, darray_t *all_focusable,
                                                  match_focus_widget_t match)
{
  uint32_t i = 0;
  point_t p = {0, 0};
  widget_t *matched = NULL;
  rect_t riter = {0, 0, 0, 0};
  rect_t rwidget = {0, 0, 0, 0};
  rect_t rmatched = {0, 0, 0, 0};

  widget_to_global(widget, &p);
  rwidget = rect_init(p.x, p.y, widget->w, widget->h);

  for (i = 0; i < all_focusable->size; i++)
  {
    widget_t *iter = WIDGET(all_focusable->elms[i]);
    if (iter == widget)
    {
      continue;
    }

    p.x = 0;
    p.y = 0;
    widget_to_global(iter, &p);

    riter = rect_init(p.x, p.y, iter->w, iter->h);
    if (match(&rwidget, (matched != NULL ? &rmatched : NULL), &riter))
    {
      matched = iter;
      rmatched = riter;
    }
  }

  return matched;
}

static widget_t *widget_find_up_focus_widget(widget_t *widget, darray_t *all_focusable)
{
  return widget_find_matched_focus_widget(widget, all_focusable, match_up);
}

static widget_t *widget_find_down_focus_widget(widget_t *widget, darray_t *all_focusable)
{
  return widget_find_matched_focus_widget(widget, all_focusable, match_down);
}

static widget_t *widget_find_left_focus_widget(widget_t *widget, darray_t *all_focusable)
{
  return widget_find_matched_focus_widget(widget, all_focusable, match_left);
}

static widget_t *widget_find_right_focus_widget(widget_t *widget, darray_t *all_focusable)
{
  return widget_find_matched_focus_widget(widget, all_focusable, match_right);
}

ret_t widget_move_focus(widget_t *widget, widget_find_wanted_focus_widget_t find)
{
  ret_t ret = RET_FAIL;
  darray_t all_focusable;

  if (widget == NULL || !widget->focused)
  {
    return RET_FAIL;
  }

  return_value_if_fail(find != NULL, RET_FAIL);
  return_value_if_fail(darray_init(&all_focusable, 10, NULL, NULL) != NULL, RET_OOM);

  widget_get_all_focusable_widgets_in_window(widget, &all_focusable);
  if (all_focusable.size > 1)
  {
    widget_t *focus = find(widget, &all_focusable);

    if (focus != NULL && focus != widget)
    {
      widget_set_prop_bool(widget, WIDGET_PROP_FOCUSED, FALSE);
      widget_set_prop_bool(focus, WIDGET_PROP_FOCUSED, TRUE);
      ret = RET_OK;
    }
  }
  darray_deinit(&all_focusable);

  return ret;
}

ret_t widget_focus_prev(widget_t *widget)
{
  return widget_move_focus(widget, widget_find_prev_focus_widget);
}

ret_t widget_focus_next(widget_t *widget)
{
  return widget_move_focus(widget, widget_find_next_focus_widget);
}

ret_t widget_focus_up(widget_t *widget)
{
  return widget_move_focus(widget, widget_find_up_focus_widget);
}

ret_t widget_focus_down(widget_t *widget)
{
  return widget_move_focus(widget, widget_find_down_focus_widget);
}

ret_t widget_focus_left(widget_t *widget)
{
  if (widget_move_focus(widget, widget_find_left_focus_widget) == RET_OK)
  {
    return RET_OK;
  }
  else
  {
    return widget_focus_up(widget);
  }
}

ret_t widget_focus_right(widget_t *widget)
{
  if (widget_move_focus(widget, widget_find_right_focus_widget) == RET_OK)
  {
    return RET_OK;
  }
  else
  {
    return widget_focus_down(widget);
  }
}

bool_t widget_is_window(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_window;
}

bool_t widget_is_designing_window(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_designing_window;
}

bool_t widget_is_window_manager(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_window_manager;
}

ret_t widget_set_need_relayout(widget_t *widget)
{
  widget_t *win = widget_get_window(widget);
  if (win != NULL)
  {
    return window_base_set_need_relayout(win, TRUE);
  }
  return RET_OK;
}

ret_t widget_set_need_relayout_children(widget_t *widget)
{
  if (widget_count_children(widget) > 0)
  {
    return widget_set_need_relayout(widget);
  }

  return RET_OK;
}

static ret_t widget_ensure_style_mutable(widget_t *widget)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (widget->astyle == NULL)
  {
    widget->astyle = style_mutable_create(NULL);
    return_value_if_fail(widget->astyle != NULL, RET_OOM);
  }
  else if (!style_is_mutable(widget->astyle))
  {
    widget->astyle = style_mutable_create(widget->astyle);
    return_value_if_fail(widget->astyle != NULL, RET_OOM);
  }

  return RET_OK;
}

ret_t widget_set_style(widget_t *widget, const char *state_and_name, const value_t *value)
{
  char str[256];
  uint32_t len = 0;
  char *name = NULL;
  char *state = NULL;
  return_value_if_fail(widget != NULL && state_and_name != NULL && value != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget_ensure_style_mutable(widget) == RET_OK, RET_BAD_PARAMS);

  len = strlen(state_and_name);
  return_value_if_fail(len < sizeof(str), RET_BAD_PARAMS);

  memcpy(str, state_and_name, len);
  str[len] = '\0';

  name = strchr(str, ':');
  if (name == NULL)
  {
    name = strchr(str, '.');
  }

  if (name != NULL)
  {
    *name++ = '\0';
    state = str;
  }
  else
  {
    name = str;
    state = WIDGET_STATE_NORMAL;
  }

  widget_invalidate(widget, NULL);

  return style_set(widget->astyle, state, name, value);
}

ret_t widget_set_style_int(widget_t *widget, const char *state_and_name, int32_t value)
{
  value_t v;
  return_value_if_fail(widget != NULL && state_and_name != NULL, RET_BAD_PARAMS);

  value_set_int(&v, value);
  return widget_set_style(widget, state_and_name, &v);
}

ret_t widget_set_style_color(widget_t *widget, const char *state_and_name, uint32_t value)
{
  value_t v;
  return_value_if_fail(widget != NULL && state_and_name != NULL, RET_BAD_PARAMS);

  value_set_int(&v, value);
  return widget_set_style(widget, state_and_name, &v);
}

ret_t widget_set_style_str(widget_t *widget, const char *state_and_name, const char *value)
{
  value_t v;
  return_value_if_fail(widget != NULL && state_and_name != NULL && value != NULL, RET_BAD_PARAMS);

  value_set_str(&v, value);
  return widget_set_style(widget, state_and_name, &v);
}

canvas_t *widget_get_canvas(widget_t *widget)
{
  canvas_t *c = NULL;
  widget_t *wm = window_manager();
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(widget != NULL, NULL);

  if (win == NULL)
  {
    win = window_manager_get_top_window(wm);
  }

  if (win != NULL)
  {
    c = (canvas_t *)widget_get_prop_pointer(win, WIDGET_PROP_CANVAS);
  }

  if (c == NULL)
  {
    c = (canvas_t *)widget_get_prop_pointer(wm, WIDGET_PROP_CANVAS);
  }

  return c;
}

bool_t widget_is_system_bar(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_window && (tk_str_eq(widget->vt->type, WIDGET_TYPE_SYSTEM_BAR) ||
                                   tk_str_eq(widget->vt->type, WIDGET_TYPE_SYSTEM_BAR_BOTTOM));
}

bool_t widget_is_normal_window(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_window && tk_str_eq(widget->vt->type, WIDGET_TYPE_NORMAL_WINDOW);
}

bool_t widget_is_dialog(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_window && tk_str_eq(widget->vt->type, WIDGET_TYPE_DIALOG);
}

bool_t widget_is_popup(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_window && tk_str_eq(widget->vt->type, WIDGET_TYPE_POPUP);
}

bool_t widget_is_overlay(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->vt != NULL, FALSE);

  return widget->vt->is_window && tk_str_eq(widget->vt->type, WIDGET_TYPE_OVERLAY);
}

bool_t widget_is_opened_dialog(widget_t *widget)
{
  int32_t stage = widget_get_prop_int(widget, WIDGET_PROP_STAGE, WINDOW_STAGE_NONE);
  return tk_str_eq(widget->vt->type, WIDGET_TYPE_DIALOG) && stage == WINDOW_STAGE_OPENED;
}

bool_t widget_is_opened_popup(widget_t *widget)
{
  int32_t stage = widget_get_prop_int(widget, WIDGET_PROP_STAGE, WINDOW_STAGE_NONE);
  return tk_str_eq(widget->vt->type, WIDGET_TYPE_POPUP) && stage == WINDOW_STAGE_OPENED;
}

ret_t widget_reset_canvas(widget_t *widget)
{
#ifndef AWTK_WEB
  rect_t rect;
  canvas_t *c = widget_get_canvas(widget);
  return_value_if_fail(c != NULL, RET_BAD_PARAMS);
  rect = rect_init(0, 0, canvas_get_width(c), canvas_get_height(c));
  canvas_set_clip_rect(c, &rect);
  canvas_reset_font(c);

  return vgcanvas_reset(canvas_get_vgcanvas(c));
#else
  (void)widget;
  return RET_OK;
#endif /*AWTK_WEB*/
}

widget_t *widget_ref(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->ref_count > 0 && widget->vt != NULL, NULL);

  widget->ref_count++;

  return widget;
}

static ret_t widget_unref_in_idle(const idle_info_t *info)
{
  widget_t *widget = WIDGET(info->ctx);
  return_value_if_fail(widget != NULL && widget->ref_count > 0 && widget->vt != NULL, RET_REMOVE);

  if (widget->ref_count > 1)
  {
    return RET_REPEAT;
  }

  widget_destroy_sync(widget);
  return RET_REMOVE;
}

ret_t widget_unref(widget_t *widget)
{
  return_value_if_fail(widget != NULL && widget->ref_count > 0 && widget->vt != NULL,
                       RET_BAD_PARAMS);

  if (widget->ref_count > 1)
  {
    widget->ref_count--;
  }
  else if (widget->ref_count == 1)
  {
    if (!idle_exist(widget_unref_in_idle, widget))
    {
      widget_destroy_sync(widget);
    }
  }

  return RET_OK;
}

static ret_t widget_unref_async(widget_t *widget)
{
  if (!idle_exist(widget_unref_in_idle, widget))
  {
    idle_add(widget_unref_in_idle, widget);
  }

  return RET_OK;
}

ret_t widget_close_window(widget_t *widget)
{
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(win != NULL, RET_BAD_PARAMS);

  return window_manager_close_window(win->parent, win);
}

ret_t widget_close_window_force(widget_t *widget)
{
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(win != NULL, RET_BAD_PARAMS);

  return window_manager_close_window_force(win->parent, win);
}

ret_t widget_back(widget_t *widget)
{
  widget_t *wm = widget_get_window_manager(widget);
  return_value_if_fail(wm != NULL, RET_BAD_PARAMS);

  return window_manager_back(wm);
}

ret_t widget_back_to_home(widget_t *widget)
{
  widget_t *wm = widget_get_window_manager(widget);
  return_value_if_fail(wm != NULL, RET_BAD_PARAMS);

  return window_manager_back_to_home(wm);
}

#if defined(FRAGMENT_FRAME_BUFFER_SIZE)
bitmap_t *widget_take_snapshot_rect(widget_t *widget, const rect_t *r)
{
  log_warn("not supported yet\n");
  return NULL;
}

#elif defined(WITH_GPU)

bitmap_t *widget_take_snapshot_rect(widget_t *widget, const rect_t *r)
{
  bitmap_t *img;
  uint32_t w = 0;
  uint32_t h = 0;
  canvas_t *c = NULL;
  vgcanvas_t *vg = NULL;
  framebuffer_object_t fbo;

  native_window_t *native_window =
      (native_window_t *)widget_get_prop_pointer(window_manager(), WIDGET_PROP_NATIVE_WINDOW);
  return_value_if_fail(native_window != NULL, NULL);

  c = native_window_get_canvas(native_window);
  vg = lcd_get_vgcanvas(c->lcd);
  return_value_if_fail(c != NULL && vg != NULL, NULL);

  vgcanvas_create_fbo(vg, vg->w, vg->h, FALSE, &fbo);
  vgcanvas_bind_fbo(vg, &fbo);
  canvas_set_clip_rect(c, r);
  canvas_translate(c, -widget->x, -widget->y);
  widget_paint(widget, c);
  canvas_translate(c, widget->x, widget->y);
  vgcanvas_unbind_fbo(vg, &fbo);

  if (r != NULL)
  {
    w = r->w;
    h = r->h;
  }
  else
  {
    w = widget->w;
    h = widget->h;
  }

  img = bitmap_create_ex(w * fbo.ratio, h * fbo.ratio, 0, BITMAP_FMT_RGBA8888);
  vgcanvas_fbo_to_bitmap(vg, &fbo, img, r);
  vgcanvas_destroy_fbo(vg, &fbo);

  return img;
}

#else
#include "../lcd/lcd_mem_rgba8888.h"
bitmap_t *widget_take_snapshot_rect(widget_t *widget, const rect_t *r)
{
  wh_t w = 0;
  wh_t h = 0;
  canvas_t canvas;
  lcd_t *lcd = NULL;
  uint8_t *buff = NULL;
  bitmap_t *bitmap = NULL;
  bitmap_t *bitmap_clip = NULL;
  return_value_if_fail(widget != NULL && widget->vt != NULL, NULL);

  w = widget->w;
  h = widget->h;

  bitmap = bitmap_create_ex(w, h, w * 4, BITMAP_FMT_RGBA8888);
  return_value_if_fail(bitmap != NULL, NULL);

  buff = bitmap_lock_buffer_for_write(bitmap);
  lcd = lcd_mem_rgba8888_create_single_fb(w, h, buff);
  if (lcd != NULL)
  {
    canvas_init(&canvas, lcd, font_manager());
    canvas_begin_frame(&canvas, NULL, LCD_DRAW_OFFLINE);
    canvas_translate(&canvas, -widget->x, -widget->y);
    widget_paint(widget, &canvas);
    canvas_translate(&canvas, widget->x, widget->y);
    canvas_end_frame(&canvas);
    canvas_reset(&canvas);
    lcd_destroy(lcd);
  }

  bitmap_unlock_buffer(bitmap);

  if (r != NULL)
  {
    rect_t widget_rect = rect_init(0, 0, widget->w, widget->h);
    rect_t clip_r = rect_intersect(&widget_rect, r);
    bitmap_clip = bitmap_create_ex(r->w, r->h, r->w * 4, BITMAP_FMT_RGBA8888);

    if (image_copy(bitmap_clip, bitmap, &clip_r, 0, 0) == RET_OK)
    {
      bitmap_destroy(bitmap);
      return bitmap_clip;
    }
    else
    {
      return NULL;
    }
  }

  return bitmap;
}
#endif /*WITH_GPU*/

bitmap_t *widget_take_snapshot(widget_t *widget)
{
  return widget_take_snapshot_rect(widget, NULL);
}

ret_t widget_dispatch_simple_event(widget_t *widget, uint32_t type)
{
  event_t e = event_init(type, widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  return widget_dispatch(widget, &e);
}

native_window_t *widget_get_native_window(widget_t *widget)
{
  widget_t *win = widget_get_window(widget);
  return_value_if_fail(win != NULL, NULL);

  return (native_window_t *)widget_get_prop_pointer(win, WIDGET_PROP_NATIVE_WINDOW);
}

ret_t widget_set_child_text_utf8(widget_t *widget, const char *name, const char *text)
{
  widget_t *child = widget_lookup(widget, name, TRUE);
  return_value_if_fail(child != NULL, RET_BAD_PARAMS);

  return widget_set_text_utf8(child, text);
}

ret_t widget_get_child_text_utf8(widget_t *widget, const char *name, char *text, uint32_t size)
{
  widget_t *child = widget_lookup(widget, name, TRUE);
  return_value_if_fail(text != NULL, RET_BAD_PARAMS);
  *text = '\0';
  return_value_if_fail(child != NULL, RET_BAD_PARAMS);

  return widget_get_text_utf8(child, text, size);
}

ret_t widget_set_child_text_with_double(widget_t *widget, const char *name, const char *format,
                                        double value)
{
  char text[128];
  widget_t *child = widget_lookup(widget, name, TRUE);
  return_value_if_fail(child != NULL && format != NULL, RET_BAD_PARAMS);

  memset(text, 0x00, sizeof(text));
  tk_snprintf(text, sizeof(text) - 1, format, value);

  return widget_set_text_utf8(child, text);
}

ret_t widget_set_child_text_with_int(widget_t *widget, const char *name, const char *format,
                                     int value)
{
  char text[128];
  widget_t *child = widget_lookup(widget, name, TRUE);
  return_value_if_fail(child != NULL && format != NULL, RET_BAD_PARAMS);

  memset(text, 0x00, sizeof(text));
  tk_snprintf(text, sizeof(text) - 1, format, value);

  return widget_set_text_utf8(child, text);
}

ret_t widget_begin_wait_pointer_cursor(widget_t *widget, bool_t ignore_user_input)
{
  widget_t *wm = widget_get_window_manager(widget);

  return window_manager_begin_wait_pointer_cursor(wm, ignore_user_input);
}

ret_t widget_end_wait_pointer_cursor(widget_t *widget)
{
  widget_t *wm = widget_get_window_manager(widget);

  return window_manager_end_wait_pointer_cursor(wm);
}

ret_t widget_draw_text_in_rect(widget_t *widget, canvas_t *c, const wchar_t *str, uint32_t size,
                               const rect_t *r, bool_t ellipses)
{
  const char *bidi_type = widget_get_bidi(widget);
  return_value_if_fail(widget != NULL && c != NULL && str != NULL && r != NULL, RET_BAD_PARAMS);

  return canvas_draw_text_bidi_in_rect(c, str, size, r, bidi_type, ellipses);
}

bool_t widget_is_parent_of(widget_t *widget, widget_t *child)
{
  widget_t *iter = NULL;
  return_value_if_fail(widget != NULL && child != NULL, FALSE);

  iter = child->parent;
  while (iter != NULL)
  {
    if (iter == widget)
    {
      return TRUE;
    }
    iter = iter->parent;
  }

  return FALSE;
}

bool_t widget_is_direct_parent_of(widget_t *widget, widget_t *child)
{
  return_value_if_fail(widget != NULL && child != NULL, FALSE);

  return child->parent == widget;
}

bool_t widget_get_enable(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->enable;
}

bool_t widget_get_floating(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->floating;
}

bool_t widget_get_auto_adjust_size(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->auto_adjust_size;
}

bool_t widget_get_with_focus_state(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->with_focus_state;
}

bool_t widget_get_focusable(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->focusable;
}

bool_t widget_get_sensitive(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->sensitive;
}

bool_t widget_get_visible(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->visible;
}

bool_t widget_get_feedback(widget_t *widget)
{
  return_value_if_fail(widget != NULL, FALSE);

  return widget->feedback;
}

rect_t widget_get_content_area(widget_t *widget)
{
  if (widget != NULL && widget->astyle != NULL)
  {
    style_t *style = widget->astyle;
    int32_t margin = style_get_int(style, STYLE_ID_MARGIN, 2);
    int32_t margin_top = style_get_int(style, STYLE_ID_MARGIN_TOP, margin);
    int32_t margin_left = style_get_int(style, STYLE_ID_MARGIN_LEFT, margin);
    int32_t margin_right = style_get_int(style, STYLE_ID_MARGIN_RIGHT, margin);
    int32_t margin_bottom = style_get_int(style, STYLE_ID_MARGIN_BOTTOM, margin);
    int32_t w = widget->w - margin_left - margin_right;
    int32_t h = widget->h - margin_top - margin_bottom;

    return rect_init(margin_left, margin_top, w, h);
  }
  else
  {
    if (widget != NULL)
    {
      return rect_init(0, 0, widget->w, widget->h);
    }
    else
    {
      return rect_init(0, 0, 0, 0);
    }
  }
}

typedef struct _auto_resize_info_t
{
  float hscale;
  float vscale;
  widget_t *widget;
  bool_t auto_scale_children_x;
  bool_t auto_scale_children_y;
  bool_t auto_scale_children_w;
  bool_t auto_scale_children_h;
} auto_resize_info_t;

static ret_t widget_auto_scale_children_child(void *ctx, const void *data)
{
  auto_resize_info_t *info = (auto_resize_info_t *)ctx;
  widget_t *widget = WIDGET(data);

  if (widget != info->widget)
  {
    if (widget->parent->children_layout == NULL && widget->self_layout == NULL)
    {
      if (info->auto_scale_children_x)
      {
        widget->x *= info->hscale;
      }
      if (info->auto_scale_children_w)
      {
        widget->w *= info->hscale;
      }
      if (info->auto_scale_children_y)
      {
        widget->y *= info->vscale;
      }
      if (info->auto_scale_children_h)
      {
        widget->h *= info->vscale;
      }
    }
  }

  return RET_OK;
}

ret_t widget_auto_scale_children(widget_t *widget, int32_t design_w, int32_t design_h,
                                 bool_t auto_scale_children_x, bool_t auto_scale_children_y,
                                 bool_t auto_scale_children_w, bool_t auto_scale_children_h)
{
  auto_resize_info_t info;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  info.widget = widget;
  info.hscale = (float)(widget->w) / (float)(design_w);
  info.vscale = (float)(widget->h) / (float)(design_h);
  info.auto_scale_children_x = auto_scale_children_x;
  info.auto_scale_children_y = auto_scale_children_y;
  info.auto_scale_children_w = auto_scale_children_w;
  info.auto_scale_children_h = auto_scale_children_h;

  widget_foreach(widget, widget_auto_scale_children_child, &info);

  return RET_OK;
}

widget_t *widget_find_parent_by_name(widget_t *widget, const char *name)
{
  widget_t *iter = NULL;
  return_value_if_fail(widget != NULL && name != NULL, NULL);

  iter = widget->parent;
  while (iter != NULL)
  {
    if (tk_str_eq(iter->name, name))
    {
      break;
    }
    iter = iter->parent;
  }

  return iter;
}

widget_t *widget_find_parent_by_type(widget_t *widget, const char *type)
{
  widget_t *iter = NULL;
  return_value_if_fail(widget != NULL && type != NULL, NULL);

  iter = widget->parent;
  while (iter != NULL)
  {
    if (tk_str_eq(widget_get_type(iter), type))
    {
      break;
    }
    iter = iter->parent;
  }

  return iter;
}

#include "object_widget.inc"

ret_t widget_set_props(widget_t *widget, const char *params)
{
  tokenizer_t t;
  const char *k = NULL;
  const char *v = NULL;
  char key[TK_NAME_LEN + 1];
  return_value_if_fail(widget != NULL && params != NULL, RET_BAD_PARAMS);

  tokenizer_init(&t, params, strlen(params), "&=");
  while (tokenizer_has_more(&t))
  {
    k = tokenizer_next(&t);
    tk_strncpy_s(key, sizeof(key) - 1, k, tk_strlen(k));

    k = key;
    v = tokenizer_next(&t);
    if (v != NULL)
    {
      widget_set_prop_str(widget, k, v);
    }
  }
  tokenizer_deinit(&t);

  return RET_OK;
}

ret_t widget_dispatch_model_event(widget_t *widget, const char *name, const char *change_type,
                                  tk_object_t *model)
{
  model_event_t event;
  event_t *e = model_event_init(&event, name, change_type, model);
  widget_t *wm = widget != NULL ? widget_get_window_manager(widget) : window_manager();
  return_value_if_fail(wm != NULL && e != NULL, RET_BAD_PARAMS);

  WIDGET_FOR_EACH_CHILD_BEGIN(wm, iter, i)
  widget_dispatch(iter, e);
  WIDGET_FOR_EACH_CHILD_END();

  return RET_OK;
}
