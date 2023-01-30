﻿/**
 * File:   pages.c
 * Author: AWTK Develop Team
 * Brief:  pages
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
 * 2018-06-16 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../tkc/mem.h"
#include "../tkc/utils.h"
#include "pages.h"
#include "../tkc/tokenizer.h"
#include "../base/widget_vtable.h"
#include "../base/image_manager.h"
#include "default_focused_child.inc"

static bool_t pages_target_is_page(widget_t *target)
{
  return target->vt != NULL && tk_str_eq(target->vt->type, WIDGET_TYPE_PAGES);
}

static bool_t pages_active_is_valid(widget_t *widget, uint32_t active)
{
  return (active < widget_count_children(widget));
}

static ret_t pages_save_target(widget_t *widget)
{
  pages_t *pages = PAGES(widget);
  return_value_if_fail(pages != NULL, RET_BAD_PARAMS);

  if (pages_active_is_valid(widget, pages->active))
  {
    widget_t *active_view = widget_get_child(widget, pages->active);

    if (active_view != NULL)
    {
      default_focused_child_set_save_target(widget, active_view, pages_target_is_page);
    }
  }
  return RET_OK;
}

static ret_t pages_on_idle_set_target_focused(const idle_info_t *idle)
{
  pages_t *pages = NULL;
  return_value_if_fail(idle != NULL, RET_BAD_PARAMS);
  pages = PAGES(idle->ctx);

  default_focused_child_set_target_focused(&(pages->str_target), WIDGET(pages));

  pages->focused_idle_id = TK_INVALID_ID;

  return RET_OK;
}

static ret_t pages_restore_target(widget_t *widget)
{
  widget_t *target = NULL;
  widget_t *active_view = NULL;
  pages_t *pages = PAGES(widget);
  return_value_if_fail(pages != NULL, RET_BAD_PARAMS);

  if (pages_active_is_valid(widget, pages->active))
  {
    active_view = widget_get_child(widget, pages->active);
  }

  if (active_view != NULL)
  {
    target = default_focused_child_get_save_target(widget, active_view);

    if (target == NULL || target->parent == NULL || target == widget)
    {
      target = active_view;
    }
    if (pages_target_is_page(target))
    {
      pages_restore_target(target);
    }
    else
    {
      default_focused_child_save_target_to_string(&(pages->str_target), target, widget);
      if (pages->focused_idle_id == TK_INVALID_ID)
      {
        pages->focused_idle_id = idle_add(pages_on_idle_set_target_focused, widget);
      }
    }
  }

  return RET_OK;
}

static ret_t pages_show_active(widget_t *widget)
{
  pages_t *pages = PAGES(widget);
  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  widget_set_visible(iter, i == pages->active);
  WIDGET_FOR_EACH_CHILD_END()

  return RET_OK;
}

ret_t pages_set_active(widget_t *widget, uint32_t index)
{
  pages_t *pages = PAGES(widget);
  return_value_if_fail(pages != NULL, RET_BAD_PARAMS);

  if (pages->active != index && widget->children != NULL)
  {
    value_change_event_t evt;

    pages_save_target(widget);
    value_change_event_init(&evt, EVT_VALUE_WILL_CHANGE, widget);
    value_set_uint32(&(evt.old_value), pages->active);
    value_set_uint32(&(evt.new_value), index);

    if (widget_dispatch(widget, (event_t *)&evt) != RET_STOP)
    {
      pages->active = index;
      pages_show_active(widget);
      evt.e.type = EVT_VALUE_CHANGED;
      widget_dispatch(widget, (event_t *)&evt);
      widget_invalidate(widget, NULL);
    }
    pages_restore_target(widget);
  }
  else
  {
    pages->active = index;
  }

  return RET_OK;
}

ret_t pages_set_active_by_name(widget_t *widget, const char *name)
{
  return_value_if_fail(widget != NULL && name != NULL, RET_BAD_PARAMS);

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (tk_str_eq(iter->name, name))
  {
    return pages_set_active(widget, i);
  }
  WIDGET_FOR_EACH_CHILD_END();

  return RET_NOT_FOUND;
}

static widget_t *pages_find_target(widget_t *widget, xy_t x, xy_t y)
{
  pages_t *pages = PAGES(widget);
  return_value_if_fail(pages != NULL, NULL);

  if (!pages_active_is_valid(widget, pages->active))
  {
    return NULL;
  }

  return widget_get_child(widget, pages->active);
}

static ret_t pages_get_prop(widget_t *widget, const char *name, value_t *v)
{
  pages_t *pages = PAGES(widget);
  return_value_if_fail(pages != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_VALUE) || tk_str_eq(name, WIDGET_PROP_ACTIVE))
  {
    value_set_uint32(v, pages->active);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t pages_set_prop(widget_t *widget, const char *name, const value_t *v)
{
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_VALUE) || tk_str_eq(name, WIDGET_PROP_ACTIVE))
  {
    return pages_set_active(widget, value_int(v));
  }

  return RET_NOT_FOUND;
}

static ret_t pages_on_event(widget_t *widget, event_t *e)
{
  pages_t *pages = PAGES(widget);
  uint16_t type = e->type;
  return_value_if_fail(widget != NULL && pages != NULL && e != NULL, RET_BAD_PARAMS);

  switch (type)
  {
  case EVT_WIDGET_ADD_CHILD:
  case EVT_WIDGET_REMOVE_CHILD:
  {
    if (widget->loading)
    {
      break;
    }
  }
  case EVT_WIDGET_LOAD:
  {
    if (!pages_active_is_valid(widget, pages->active))
    {
      pages_set_active(widget, 0);
    }
    else
    {
      pages_show_active(widget);
    }
    break;
  }
  case EVT_BLUR:
  {
    pages_save_target(widget);
    break;
  }
  }

  return RET_OK;
}

static ret_t pages_on_idle_init_save_target(const idle_info_t *idle)
{
  pages_t *pages = NULL;
  return_value_if_fail(idle != NULL, RET_BAD_PARAMS);
  pages = PAGES(idle->ctx);

  pages_restore_target(WIDGET(pages));
  pages->init_idle_id = TK_INVALID_ID;

  return RET_OK;
}

static ret_t pages_on_destroy(widget_t *widget)
{
  pages_t *pages = PAGES(widget);
  return_value_if_fail(pages != NULL, RET_BAD_PARAMS);

  if (pages->init_idle_id != TK_INVALID_ID)
  {
    idle_remove(pages->init_idle_id);
  }
  if (pages->focused_idle_id != TK_INVALID_ID)
  {
    idle_remove(pages->focused_idle_id);
  }

  str_reset(&(pages->str_target));

  return RET_OK;
}

static ret_t pages_on_add_child(widget_t *widget, widget_t *child)
{
  widget_add_child_default(widget, child);
  pages_show_active(widget);
  return RET_OK;
}

static ret_t pages_on_remove_child(widget_t *widget, widget_t *child)
{
  pages_t *pages = PAGES(widget);
  return_value_if_fail(widget != NULL && pages != NULL && child != NULL, RET_BAD_PARAMS);

  if (!widget->destroying)
  {
    int32_t active = (int32_t)(pages->active);
    int32_t remove_index = widget_index_of(child);
    return_value_if_fail(remove_index >= 0, RET_BAD_PARAMS);

    if (remove_index < active ||
        (remove_index == active && remove_index == widget->children->size - 1))
    {
      active = tk_max(active - 1, 0);
      pages->active = (uint32_t)active;
    }
  }
  return RET_CONTINUE;
}

static ret_t pages_get_only_active_children(widget_t *widget, darray_t *all_focusable)
{
  widget_t *child = widget_find_target(widget, 1, 1);
  return_value_if_fail(widget != NULL && all_focusable != NULL, RET_BAD_PARAMS);
  widget_foreach(child, widget_on_visit_focusable, all_focusable);

  return RET_SKIP;
}

static const char *const s_pages_clone_properties[] = {WIDGET_PROP_VALUE, NULL};

TK_DECL_VTABLE(pages) = {.size = sizeof(pages_t),
                         .inputable = TRUE,
                         .type = WIDGET_TYPE_PAGES,
                         .get_only_active_children = pages_get_only_active_children,
                         .clone_properties = s_pages_clone_properties,
                         .get_parent_vt = TK_GET_PARENT_VTABLE(widget),
                         .create = pages_create,
                         .on_paint_self = widget_on_paint_null,
                         .on_paint_children = widget_on_paint_children_clip,
                         .find_target = pages_find_target,
                         .on_event = pages_on_event,
                         .get_prop = pages_get_prop,
                         .set_prop = pages_set_prop,
                         .on_add_child = pages_on_add_child,
                         .on_remove_child = pages_on_remove_child,
                         .on_destroy = pages_on_destroy};

widget_t *pages_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
  widget_t *widget = widget_create(parent, TK_REF_VTABLE(pages), x, y, w, h);
  pages_t *pages = PAGES(widget);
  return_value_if_fail(pages != NULL, NULL);
  str_init(&(pages->str_target), DEFAULT_FOCUSED_CHILD_SAVE_TARGET_TAG_LENGT);
  pages->init_idle_id = idle_add(pages_on_idle_init_save_target, widget);
  pages->active = 0xffffffff;

  return widget;
}

widget_t *pages_cast(widget_t *widget)
{
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, pages), NULL);

  return widget;
}
