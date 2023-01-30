﻿/**
 * File:   combo_box_item.h
 * Author: AWTK Develop Team
 * Brief:  combo_box_item
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
 * 2018-07-25 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../tkc/mem.h"
#include "combo_box_item.h"

static ret_t combo_box_item_on_paint_self(widget_t *widget, canvas_t *c)
{
  return widget_paint_helper(widget, c, NULL, NULL);
}

static ret_t combo_box_item_on_event(widget_t *widget, event_t *e)
{
  ret_t ret = RET_OK;
  uint16_t type = e->type;
  combo_box_item_t *combo_box_item = COMBO_BOX_ITEM(widget);
  return_value_if_fail(combo_box_item != NULL, RET_BAD_PARAMS);

  switch (type)
  {
  case EVT_POINTER_DOWN:
  {
    combo_box_item->pressed = TRUE;
    widget_set_state(widget, WIDGET_STATE_PRESSED);
    break;
  }
  case EVT_POINTER_DOWN_ABORT:
  {
    combo_box_item->pressed = FALSE;
    widget_set_state(widget, WIDGET_STATE_NORMAL);
    break;
  }
  case EVT_POINTER_UP:
  {
    pointer_event_t evt = *(pointer_event_t *)e;
    if (combo_box_item->pressed)
    {
      evt.e = event_init(EVT_CLICK, widget);
      evt.e.size = sizeof(pointer_event_t);
      combo_box_item_set_checked(widget, TRUE);
      ret = widget_dispatch(widget, (event_t *)&evt);
    }

    combo_box_item->pressed = FALSE;
    widget_set_state(widget, WIDGET_STATE_NORMAL);
    break;
  }
  case EVT_POINTER_LEAVE:
    combo_box_item->pressed = FALSE;
    widget_set_state(widget, WIDGET_STATE_NORMAL);
    break;
  case EVT_POINTER_ENTER:
    widget_set_state(widget, WIDGET_STATE_OVER);
    break;
  default:
    break;
  }

  return ret;
}

static ret_t combo_box_item_set_prop(widget_t *widget, const char *name, const value_t *v)
{
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_VALUE))
  {
    return combo_box_item_set_value(widget, value_int(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_CHECKED))
  {
    return combo_box_item_set_checked(widget, value_bool(v));
  }

  return RET_NOT_FOUND;
}

static ret_t combo_box_item_get_prop(widget_t *widget, const char *name, value_t *v)
{
  combo_box_item_t *combo_box_item = COMBO_BOX_ITEM(widget);
  return_value_if_fail(combo_box_item != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_VALUE))
  {
    value_set_int(v, combo_box_item->value);
    return RET_OK;
  }
  else if (tk_str_eq(name, WIDGET_PROP_CHECKED))
  {
    value_set_bool(v, combo_box_item->checked);
    return RET_OK;
  }
  else if (tk_str_eq(name, WIDGET_PROP_STATE_FOR_STYLE))
  {
    value_set_str(v, widget_get_state_for_style(widget, FALSE, combo_box_item->checked));
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

TK_DECL_VTABLE(combo_box_item) = {.size = sizeof(combo_box_item_t),
                                  .type = WIDGET_TYPE_COMBO_BOX_ITEM,
                                  .focusable = TRUE,
                                  .space_key_to_activate = TRUE,
                                  .return_key_to_activate = TRUE,
                                  .on_paint_self = combo_box_item_on_paint_self,
                                  .on_event = combo_box_item_on_event,
                                  .get_prop = combo_box_item_get_prop,
                                  .set_prop = combo_box_item_set_prop,
                                  .get_parent_vt = TK_GET_PARENT_VTABLE(widget),
                                  .create = combo_box_item_create};

widget_t *combo_box_item_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
  return widget_create(parent, TK_REF_VTABLE(combo_box_item), x, y, w, h);
}

ret_t combo_box_item_set_value(widget_t *widget, int32_t value)
{
  combo_box_item_t *combo_box_item = COMBO_BOX_ITEM(widget);
  return_value_if_fail(combo_box_item != NULL, RET_BAD_PARAMS);

  combo_box_item->value = value;

  return RET_OK;
}

static ret_t combo_box_item_set_checked_only(widget_t *widget, bool_t checked)
{
  combo_box_item_t *combo_box_item = COMBO_BOX_ITEM(widget);
  return_value_if_fail(combo_box_item != NULL, RET_BAD_PARAMS);

  if (combo_box_item->checked != checked)
  {
    value_change_event_t evt;
    value_change_event_init(&evt, EVT_VALUE_WILL_CHANGE, widget);
    value_set_bool(&(evt.old_value), combo_box_item->checked);
    value_set_bool(&(evt.new_value), checked);

    if (widget_dispatch(widget, (event_t *)&evt) != RET_STOP)
    {
      combo_box_item->checked = checked;
      evt.e.type = EVT_VALUE_CHANGED;
      widget_dispatch(widget, (event_t *)&evt);
      widget_invalidate(widget, NULL);
    }
  }

  widget_set_need_update_style(widget);

  return RET_OK;
}

ret_t combo_box_item_set_checked(widget_t *widget, bool_t checked)
{
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  combo_box_item_set_checked_only(widget, checked);
  if (widget->parent != NULL && checked)
  {
    widget_t *parent = widget->parent;

    WIDGET_FOR_EACH_CHILD_BEGIN(parent, iter, i)
    if (iter != widget && iter->vt == widget->vt)
    {
      combo_box_item_set_checked_only(iter, !checked);
    }
    WIDGET_FOR_EACH_CHILD_END();
  }

  return RET_OK;
}

widget_t *combo_box_item_cast(widget_t *widget)
{
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, combo_box_item), NULL);

  return widget;
}
