﻿/**
 * File:   hscroll_label.h
 * Author: AWTK Develop Team
 * Brief:  hscroll_label
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
 * 2019-07-05 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../../tkc/mem.h"
#include "../../tkc/utils.h"
#include "../../tkc/time_now.h"
#include "../../base/widget_vtable.h"
#include "hscroll_label.h"

#ifndef HSCROLL_LABEL_STOP_TIMER_DURATION
#define HSCROLL_LABEL_STOP_TIMER_DURATION 500
#endif

static ret_t hscroll_label_remove_timer(widget_t *widget);
static ret_t hscroll_label_check_and_start(widget_t *widget);

static bool_t hscroll_label_is_running(widget_t *widget)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  return hscroll_label->timer_id != TK_INVALID_ID;
}

static ret_t hscroll_label_do_paint_self(widget_t *widget, canvas_t *c, uint32_t left_margin,
                                         uint32_t right_margin)
{
  rect_t r = {0, 0, 0, 0};
  wstr_t *text = &(widget->text);
  uint32_t w = widget->w - left_margin - right_margin;
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);

  hscroll_label->text_w = canvas_measure_text(c, text->str, text->size);
  if (hscroll_label->text_w != hscroll_label->old_text_w)
  {
    if (tk_str_eq(widget->state, WIDGET_STATE_FOCUSED))
    {
      hscroll_label_start(widget);
    }
    else
    {
      hscroll_label_check_and_start(widget);
    }
    hscroll_label->old_text_w = hscroll_label->text_w;
  }
  if (w < hscroll_label->text_w && hscroll_label->ellipses && !hscroll_label->yoyo &&
      !hscroll_label_is_running(widget))
  {
    r = rect_init(left_margin, 0, w, widget->h);

    return widget_draw_text_in_rect(widget, c, text->str, text->size, &r, TRUE);
  }

  if (w > hscroll_label->text_w)
  {
    int32_t align_v = style_get_int(widget->astyle, STYLE_ID_TEXT_ALIGN_V, ALIGN_V_MIDDLE);
    int32_t align_h = style_get_int(widget->astyle, STYLE_ID_TEXT_ALIGN_H, ALIGN_H_LEFT);
    canvas_set_text_align(c, (align_h_t)align_h, (align_v_t)align_v);
  }
  else
  {
    canvas_set_text_align(c, ALIGN_H_LEFT, ALIGN_V_MIDDLE);
  }

  r = rect_init(left_margin - hscroll_label->xoffset, 0, w, widget->h);
  return widget_draw_text_in_rect(widget, c, text->str, text->size, &r, FALSE);
}

static ret_t hscroll_label_on_paint_self(widget_t *widget, canvas_t *c)
{
  rect_t save_r = {0, 0, 0, 0};
  rect_t clip_r = {0, 0, 0, 0};
  rect_t widget_r = {0, 0, 0, 0};

  if (widget->text.size > 0)
  {
    style_t *style = widget->astyle;
    color_t black = color_init(0, 0, 0, 0xff);
    const char *font = style_get_str(style, STYLE_ID_FONT_NAME, NULL);
    int32_t left_margin = style_get_int(style, STYLE_ID_MARGIN_LEFT, 2);
    int32_t right_margin = style_get_int(style, STYLE_ID_MARGIN_RIGHT, 2);
    int32_t font_size = style_get_int(style, STYLE_ID_FONT_SIZE, TK_DEFAULT_FONT_SIZE);
    color_t text_color = style_get_color(style, STYLE_ID_TEXT_COLOR, black);
    int32_t w = widget->w - left_margin - right_margin;

    canvas_get_clip_rect(c, &save_r);
    widget_r = rect_init(c->ox + left_margin, c->oy, w, widget->h);
    clip_r = rect_intersect(&save_r, &widget_r);

    canvas_set_clip_rect(c, &clip_r);
    canvas_set_font(c, font, font_size);
    canvas_set_text_color(c, text_color);
    // canvas_set_text_align(c, ALIGN_H_LEFT, ALIGN_V_MIDDLE);

    hscroll_label_do_paint_self(widget, c, left_margin, right_margin);
    canvas_set_clip_rect(c, &save_r);
  }

  return RET_OK;
}

ret_t hscroll_label_set_xoffset(widget_t *widget, int32_t xoffset)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->xoffset = xoffset;

  return widget_invalidate_force(widget, NULL);
}

ret_t hscroll_label_set_lull(widget_t *widget, int32_t lull)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->lull = lull;

  return RET_OK;
}

ret_t hscroll_label_set_duration(widget_t *widget, int32_t duration)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->duration = duration;

  return RET_OK;
}

ret_t hscroll_label_set_speed(widget_t *widget, float_t speed)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);
  return_value_if_fail(speed >= 0, RET_BAD_PARAMS);

  hscroll_label->speed = speed;

  return RET_OK;
}

ret_t hscroll_label_set_loop(widget_t *widget, bool_t loop)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->loop = loop;

  return RET_OK;
}

ret_t hscroll_label_set_only_focus(widget_t *widget, bool_t only_focus)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->only_focus = only_focus;

  return RET_OK;
}

ret_t hscroll_label_set_only_parent_focus(widget_t *widget, bool_t only_parent_focus)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->only_parent_focus = only_parent_focus;

  return RET_OK;
}

ret_t hscroll_label_set_yoyo(widget_t *widget, bool_t yoyo)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->yoyo = yoyo;

  return RET_OK;
}

ret_t hscroll_label_set_ellipses(widget_t *widget, bool_t ellipses)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->ellipses = ellipses;

  return RET_OK;
}

ret_t hscroll_label_set_stop_at_begin(widget_t *widget, bool_t stop_at_begin)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->stop_at_begin = stop_at_begin;

  return RET_OK;
}

static ret_t hscroll_label_get_prop(widget_t *widget, const char *name, value_t *v)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, HSCROLL_LABEL_PROP_LOOP))
  {
    value_set_bool(v, hscroll_label->loop);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_LULL))
  {
    value_set_int(v, hscroll_label->lull);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_DURATION))
  {
    value_set_int(v, hscroll_label->duration);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_SPEED))
  {
    value_set_float32(v, hscroll_label->speed);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_XOFFSET))
  {
    value_set_int(v, hscroll_label->xoffset);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_YOYO))
  {
    value_set_bool(v, hscroll_label->yoyo);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_ELLIPSES))
  {
    value_set_bool(v, hscroll_label->ellipses);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_ONLY_FOCUS))
  {
    value_set_bool(v, hscroll_label->only_focus);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_ONLY_PARENT_FOCUS))
  {
    value_set_bool(v, hscroll_label->only_parent_focus);
    return RET_OK;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_STOP_AT_BEGIN))
  {
    value_set_bool(v, hscroll_label->stop_at_begin);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t hscroll_label_set_prop(widget_t *widget, const char *name, const value_t *v)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, HSCROLL_LABEL_PROP_LOOP))
  {
    return hscroll_label_set_loop(widget, value_bool(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_LULL))
  {
    return hscroll_label_set_lull(widget, value_int(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_DURATION))
  {
    return hscroll_label_set_duration(widget, value_int(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_SPEED))
  {
    return hscroll_label_set_speed(widget, value_float32(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_XOFFSET))
  {
    return hscroll_label_set_xoffset(widget, value_int(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_YOYO))
  {
    return hscroll_label_set_yoyo(widget, value_bool(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_ELLIPSES))
  {
    return hscroll_label_set_ellipses(widget, value_bool(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_ONLY_FOCUS))
  {
    return hscroll_label_set_only_focus(widget, value_bool(v));
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_ONLY_PARENT_FOCUS))
  {
    return hscroll_label_set_only_parent_focus(widget, value_bool(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_TEXT))
  {
    if (hscroll_label->timer_id != TK_INVALID_ID)
    {
      hscroll_label->elapsed = 0;
    }
    return RET_NOT_FOUND;
  }
  else if (tk_str_eq(name, HSCROLL_LABEL_PROP_STOP_AT_BEGIN))
  {
    return hscroll_label_set_stop_at_begin(widget, value_bool(v));
  }

  return RET_NOT_FOUND;
}

static int32_t hscroll_label_get_range(widget_t *widget)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  int32_t left_margin = style_get_int(widget->astyle, STYLE_ID_MARGIN_LEFT, 2);
  int32_t right_margin = style_get_int(widget->astyle, STYLE_ID_MARGIN_RIGHT, 2);
  int32_t w = widget->w - left_margin - right_margin;

  return hscroll_label->text_w - w;
}

ret_t hscroll_label_step(widget_t *widget)
{
  ret_t ret = RET_REPEAT;
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  float_t percent = 0;
  uint32_t duration = 0;
  int32_t range = hscroll_label_get_range(widget);

  if (range < 0)
  {
    range = 0;
    ret = RET_REMOVE;
  }

  duration = (hscroll_label->speed > 0 && range > 0) ? (range / hscroll_label->speed)
                                                     : hscroll_label->duration;

  if (hscroll_label->elapsed >= duration)
  {
    hscroll_label->elapsed = duration;
  }

  percent = (float_t)hscroll_label->elapsed / (float_t)(duration);
  if (hscroll_label->reversed)
  {
    percent = 1 - percent;
  }

  hscroll_label->xoffset = range * percent;
  if (hscroll_label->xoffset > range)
  {
    ret = RET_REMOVE;
  }

  widget_invalidate_force(widget, NULL);

  if (ret == RET_REMOVE)
  {
    hscroll_label_remove_timer(widget);
  }
  return ret;
}

static ret_t hscroll_label_check_and_start(widget_t *widget)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(widget != NULL && hscroll_label != NULL, RET_BAD_PARAMS);
  if (hscroll_label->only_focus)
  {
    if (widget->focused)
    {
      hscroll_label_start(widget);
    }
  }
  else if (hscroll_label->only_parent_focus)
  {
    if (widget->parent->focused)
    {
      hscroll_label_start(widget);
    }
  }
  else
  {
    hscroll_label_start(widget);
  }
  return RET_OK;
}

static ret_t hscroll_label_on_timer_start(const timer_info_t *info)
{
  widget_t *widget = WIDGET(info->ctx);
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);
  hscroll_label->timer_id = TK_INVALID_ID;

  hscroll_label_check_and_start(widget);
  widget_invalidate_force(widget, NULL);

  return RET_REMOVE;
}

static ret_t hscroll_label_on_timer_stop(const timer_info_t *info)
{
  widget_t *widget = WIDGET(info->ctx);
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label_stop(widget);

  return RET_REMOVE;
}

static ret_t hscroll_label_on_timer(const timer_info_t *info)
{
  ret_t ret = RET_OK;
  widget_t *widget = WIDGET(info->ctx);
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  uint32_t duration = 0;
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);
  if (!hscroll_label->paused)
  {
    hscroll_label->elapsed += info->duration;
  }
  else
  {
    return RET_REPEAT;
  }

  if (hscroll_label->only_focus)
  {
    if (!widget->focused)
    {
      hscroll_label_stop(widget);

      return RET_REMOVE;
    }
  }
  else if (hscroll_label->only_parent_focus)
  {
    if (!widget->parent->focused)
    {
      hscroll_label_stop(widget);

      return RET_REMOVE;
    }
  }

  ret = hscroll_label_step(widget);

  if (hscroll_label->speed > 0)
  {
    int32_t range = hscroll_label_get_range(widget);
    if (range > 0)
    {
      duration = range / hscroll_label->speed;
    }
  }
  else
  {
    duration = hscroll_label->duration;
  }

  if (hscroll_label->elapsed >= duration)
  {
    uint32_t lull = hscroll_label->lull;

    hscroll_label->elapsed = 0;

    if (!hscroll_label->loop)
    {
      ret = RET_REMOVE;
      hscroll_label_remove_timer(widget);
      if (hscroll_label->stop_at_begin)
      {
        widget_add_timer(widget, hscroll_label_on_timer_stop, HSCROLL_LABEL_STOP_TIMER_DURATION);
      }
    }
    else
    {
      if (hscroll_label->lull > 0)
      {
        ret = RET_REMOVE;
        hscroll_label_remove_timer(widget);
        hscroll_label->timer_id = widget_add_timer(widget, hscroll_label_on_timer_start, lull);
      }
    }

    if (hscroll_label->yoyo)
    {
      hscroll_label->reversed = !hscroll_label->reversed;
    }
  }

  return ret;
}

ret_t hscroll_label_start(widget_t *widget)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);
  return_value_if_fail(hscroll_label->timer_id == TK_INVALID_ID, RET_BAD_PARAMS);

  hscroll_label->elapsed = 0;
  hscroll_label->timer_id = widget_add_timer(widget, hscroll_label_on_timer, 16);

  return RET_OK;
}

static ret_t hscroll_label_remove_timer(widget_t *widget)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  if (hscroll_label->timer_id != TK_INVALID_ID)
  {
    timer_remove(hscroll_label->timer_id);
    hscroll_label->timer_id = TK_INVALID_ID;
  }

  return RET_OK;
}

ret_t hscroll_label_stop(widget_t *widget)
{
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);

  hscroll_label->xoffset = 0;
  hscroll_label->reversed = FALSE;

  return hscroll_label_remove_timer(widget);
}

static ret_t hscroll_label_on_destroy(widget_t *widget)
{
  if (hscroll_label_is_running(widget))
  {
    hscroll_label_stop(widget);
  }

  return RET_OK;
}

static ret_t hscroll_label_on_event(widget_t *widget, event_t *e)
{
  uint16_t type = e->type;
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL && widget != NULL, RET_BAD_PARAMS);

  switch (type)
  {
  case EVT_FOCUS:
  {
    if (!hscroll_label_is_running(widget))
    {
      hscroll_label_start(widget);
    }
    break;
  }
  case EVT_BLUR:
    if (!hscroll_label->only_focus)
    {
      break;
    }
  case EVT_POINTER_DOWN_ABORT:
  {
    if (hscroll_label_is_running(widget))
    {
      hscroll_label_stop(widget);
    }
    break;
  }
  case EVT_POINTER_ENTER:
  {
    if (!hscroll_label->only_focus && !hscroll_label->only_parent_focus)
    {
      hscroll_label->paused = TRUE;
    }
    break;
  }
  case EVT_POINTER_UP:
  {
    hscroll_label->paused = FALSE;
    break;
  }
  case EVT_POINTER_LEAVE:
  {
    if (!hscroll_label->only_focus && !hscroll_label->only_parent_focus)
    {
      hscroll_label->paused = FALSE;
    }
    break;
  }
  default:
    break;
  }

  return RET_OK;
}

static ret_t hscroll_label_on_parent_focus_changed(void *ctx, event_t *e)
{
  widget_t *widget = WIDGET(ctx);
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, RET_BAD_PARAMS);
  if (hscroll_label->only_parent_focus)
  {
    if (e->type == EVT_FOCUS)
    {
      hscroll_label_start(widget);
      widget_set_state(widget, WIDGET_STATE_FOCUSED);
    }
    else
    {
      hscroll_label_stop(widget);
      widget_set_state(widget, WIDGET_STATE_NORMAL);
    }
  }

  return RET_OK;
}

static ret_t hscroll_label_on_attach_parent(widget_t *widget, widget_t *parent)
{
  widget_on_with_tag(parent, EVT_FOCUS, hscroll_label_on_parent_focus_changed, widget,
                     tk_pointer_to_int(widget));
  widget_on_with_tag(parent, EVT_BLUR, hscroll_label_on_parent_focus_changed, widget,
                     tk_pointer_to_int(widget));

  return RET_OK;
}

static ret_t hscroll_label_on_detach_parent(widget_t *widget, widget_t *parent)
{
  widget_off_by_tag(parent, tk_pointer_to_int(widget));

  return RET_OK;
}

static const char *const s_hscroll_label_properties[] = {HSCROLL_LABEL_PROP_YOYO,
                                                         HSCROLL_LABEL_PROP_LOOP,
                                                         HSCROLL_LABEL_PROP_LULL,
                                                         HSCROLL_LABEL_PROP_DURATION,
                                                         HSCROLL_LABEL_PROP_SPEED,
                                                         HSCROLL_LABEL_PROP_ELLIPSES,
                                                         HSCROLL_LABEL_PROP_ONLY_FOCUS,
                                                         HSCROLL_LABEL_PROP_ONLY_PARENT_FOCUS,
                                                         NULL};

TK_DECL_VTABLE(hscroll_label) = {.size = sizeof(hscroll_label_t),
                                 .type = WIDGET_TYPE_HSCROLL_LABEL,
                                 .clone_properties = s_hscroll_label_properties,
                                 .get_parent_vt = TK_GET_PARENT_VTABLE(widget),
                                 .create = hscroll_label_create,
                                 .on_destroy = hscroll_label_on_destroy,
                                 .on_attach_parent = hscroll_label_on_attach_parent,
                                 .on_detach_parent = hscroll_label_on_detach_parent,
                                 .set_prop = hscroll_label_set_prop,
                                 .get_prop = hscroll_label_get_prop,
                                 .on_event = hscroll_label_on_event,
                                 .on_paint_self = hscroll_label_on_paint_self};

widget_t *hscroll_label_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
  widget_t *widget = widget_create(parent, TK_REF_VTABLE(hscroll_label), x, y, w, h);
  hscroll_label_t *hscroll_label = HSCROLL_LABEL(widget);
  return_value_if_fail(hscroll_label != NULL, NULL);

  hscroll_label->xoffset = 0;
  hscroll_label->lull = 3000;
  hscroll_label->duration = 5000;

  hscroll_label->loop = FALSE;
  hscroll_label->yoyo = FALSE;
  hscroll_label->ellipses = FALSE;
  hscroll_label->only_focus = FALSE;
  hscroll_label->stop_at_begin = FALSE;

  hscroll_label->paused = FALSE;

  hscroll_label->timer_id = TK_INVALID_ID;

  return widget;
}

widget_t *hscroll_label_cast(widget_t *widget)
{
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, hscroll_label), NULL);

  return widget;
}
