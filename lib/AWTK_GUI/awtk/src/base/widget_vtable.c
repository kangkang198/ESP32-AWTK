﻿/**
 * File:   widget_vtable.c
 * Author: AWTK Develop Team
 * Brief:  widget vtable default impl
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
 * 2018-01-26 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "widget_vtable.h"
#include "../tkc/mem.h"

widget_vtable_t *widget_vtable_init(widget_vtable_t *vt, const widget_vtable_t *parent)
{
  return_value_if_fail(vt != NULL && parent != NULL, NULL);
  tk_memcpy(vt, parent, sizeof(widget_vtable_t));
  vt->parent = parent;
  vt->get_parent_vt = NULL;
  return vt;
}

const widget_vtable_t *widget_get_parent_vtable(const widget_vtable_t *vt)
{
  return_value_if_fail(vt != NULL, NULL);
  return_value_if_fail(!(vt->parent != NULL && vt->get_parent_vt != NULL), NULL);

  if (vt->parent != NULL)
  {
    return vt->parent;
  }
  if (vt->get_parent_vt != NULL)
  {
    return vt->get_parent_vt();
  }
  return NULL;
}

ret_t widget_set_self_vtable(widget_t *widget, const widget_vtable_t *vt)
{
  return_value_if_fail(widget != NULL && vt != NULL, RET_BAD_PARAMS);
  widget->vt = vt;
  return RET_OK;
}

const widget_vtable_t *widget_get_self_vtable(widget_t *widget)
{
  return_value_if_fail(widget != NULL, NULL);
  return widget->vt;
}

ret_t widget_invalidate_default(widget_t *widget, const rect_t *rect)
{
  rect_t t = *rect;
  rect_t *r = &t;
  if (widget->vt->scrollable)
  {
    rect_t r_self = rect_init(0, 0, widget->w, widget->h);

    if (!widget->dirty)
    {
      int32_t ox = 0, oy = 0;
      widget_get_offset(widget, &ox, &oy);
      if (ox > 0)
      {
        r->x -= ox;
      }
      if (oy > 0)
      {
        r->y -= oy;
      }
    }

    *r = rect_intersect(r, &r_self);
  }

  if (r->w <= 0 || r->h <= 0)
  {
    return RET_OK;
  }

  r->x += widget->x;
  r->y += widget->y;

  if (widget->astyle != NULL)
  {
    int32_t tolerance = widget->dirty_rect_tolerance;

    if (tolerance > 0)
    {
      r->x -= tolerance;
      r->y -= tolerance;
      r->w += 2 * tolerance + 1;
      r->h += 2 * tolerance + 1;
    }
  }

  if (r->x < 0)
  {
    r->w = r->w + r->x;
  }
  if (r->y < 0)
  {
    r->h = r->h + r->y;
  }

  r->x = tk_max(0, r->x);
  r->y = tk_max(0, r->y);
  r->w = tk_max(0, r->w);
  r->h = tk_max(0, r->h);

  if (widget->parent)
  {
    widget_t *parent = widget->parent;
    if (parent->vt && parent->vt->invalidate)
    {
      parent->vt->invalidate(parent, r);
    }
    else
    {
      widget_invalidate_default(parent, r);
    }
  }

  return RET_OK;
}

ret_t widget_on_event_default(widget_t *widget, event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  return RET_OK;
}

ret_t widget_on_paint_self_default(widget_t *widget, canvas_t *c)
{
  if (style_is_valid(widget->astyle))
  {
    return widget_paint_helper(widget, c, NULL, NULL);
  }
  return RET_OK;
}

ret_t widget_on_paint_children_default(widget_t *widget, canvas_t *c)
{
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)

  if (!iter->visible)
  {
    iter->dirty = FALSE;
    continue;
  }

  if (!(iter->vt->allow_draw_outside))
  {
    int32_t tolerance = iter->dirty_rect_tolerance;
    int32_t left = c->ox + iter->x - tolerance;
    int32_t top = c->oy + iter->y - tolerance;
    int32_t bottom = top + iter->h + 2 * tolerance;
    int32_t right = left + iter->w + 2 * tolerance;

    if (!canvas_is_rect_in_clip_rect(c, left, top, right, bottom))
    {
      iter->dirty = FALSE;
      continue;
    }
  }

  widget_paint(iter, c);
  WIDGET_FOR_EACH_CHILD_END();

  return RET_OK;
}

ret_t widget_on_keydown_default(widget_t *widget, key_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  return RET_OK;
}

ret_t widget_on_keyup_default(widget_t *widget, key_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  return RET_OK;
}

ret_t widget_on_pointer_down_default(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  return RET_OK;
}

ret_t widget_on_pointer_move_default(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  return RET_OK;
}

ret_t widget_on_pointer_up_default(widget_t *widget, pointer_event_t *e)
{
  return_value_if_fail(widget != NULL && e != NULL, RET_BAD_PARAMS);

  return RET_OK;
}

ret_t widget_get_prop_default(widget_t *widget, const char *name, value_t *v)
{
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  return RET_NOT_FOUND;
}

ret_t widget_set_prop_default(widget_t *widget, const char *name, const value_t *v)
{
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  return RET_NOT_FOUND;
}

widget_t *widget_find_target_default(widget_t *widget, xy_t x, xy_t y)
{
  xy_t xx = 0;
  xy_t yy = 0;
  point_t p = {x, y};
  return_value_if_fail(widget != NULL, NULL);

  if (widget->grab_widget != NULL)
  {
    return widget->grab_widget;
  }

  widget_to_local(widget, &p);
  WIDGET_FOR_EACH_CHILD_BEGIN_R(widget, iter, i)
  if (!iter->sensitive || !iter->enable)
  {
    continue;
  }

  xx = p.x - iter->x;
  yy = p.y - iter->y;
  if (!widget_is_point_in(iter, xx, yy, TRUE))
  {
    continue;
  }
  return iter;
  WIDGET_FOR_EACH_CHILD_END();

  return NULL;
}

ret_t widget_copy_props(widget_t *dst, widget_t *src, const char *const *properties)
{
  if (properties != NULL)
  {
    value_t v;
    value_t defval;
    uint32_t i = 0;
    for (i = 0; properties[i] != NULL; i++)
    {
      const char *prop = properties[i];
      if (widget_get_prop(src, prop, &v) == RET_OK)
      {
        if (widget_get_prop_default_value(src, prop, &defval) == RET_OK)
        {
          if (!value_equal(&v, &defval))
          {
            widget_set_prop(dst, prop, &v);
          }
        }
        else
        {
          widget_set_prop(dst, prop, &v);
        }
      }
    }
  }

  return RET_OK;
}

ret_t widget_on_copy_default(widget_t *widget, widget_t *other)
{
  return widget_copy_props(widget, other, widget->vt->clone_properties);
}

ret_t widget_on_destroy_default(widget_t *widget)
{
  (void)widget;
  return RET_OK;
}

ret_t widget_on_paint_null(widget_t *widget, canvas_t *c)
{
  (void)widget;
  (void)c;
  return RET_OK;
}

ret_t widget_paint_with_clip(widget_t *widget, rect_t *clip, canvas_t *c,
                             widget_on_paint_t on_paint)
{
  rect_t r = {0};
  rect_t r_save = {0};
  rect_t r_vg_save = {0};
  vgcanvas_t *vg = canvas_get_vgcanvas(c);
  return_value_if_fail(widget != NULL && on_paint != NULL, RET_BAD_PARAMS);

  /* 裁剪子控件的话，需要注意保存和还原 canvas 和 vg 这两个画布，*/
  /* 因为子控件可能会修改任意一个画布的裁剪区或者其他的配置，有概率会导致其他的控件的绘图不正常 */
  /* 如果创建新的裁剪区的话，需要和之前的裁剪区做交集，让新的裁剪区必须在旧的裁剪区中 */
  canvas_save(c);
  canvas_get_clip_rect(c, &r_save);
  if (vg != NULL)
  {
    vgcanvas_save(vg);
    r_vg_save = rect_from_rectf(vgcanvas_get_clip_rect(vg));
  }

  if (clip != NULL)
  {
    r = *clip;
  }
  else
  {
    r = rect_init(c->ox, c->oy, widget->w, widget->h);
  }
  r = rect_intersect(&r, &r_save);
  canvas_set_clip_rect(c, &r);
  if (vg != NULL)
  {
    vgcanvas_clip_rect(vg, r.x, r.y, r.w, r.h);
  }

  on_paint(widget, c);

  if (vg != NULL)
  {
    vgcanvas_clip_rect(vg, r_vg_save.x, r_vg_save.y, r_vg_save.w, r_vg_save.h);
    vgcanvas_restore(vg);
  }
  canvas_set_clip_rect(c, &r_save);
  canvas_restore(c);

  return RET_OK;
}

ret_t widget_on_paint_children_clip(widget_t *widget, canvas_t *c)
{
  return widget_paint_with_clip(widget, NULL, c, widget_on_paint_children_default);
}

TK_DECL_VTABLE(widget) = {.size = sizeof(widget_t),
                          .type = WIDGET_TYPE_NONE,
                          .parent = NULL,
                          .get_parent_vt = NULL,
                          .on_copy = widget_on_copy_default,
                          .invalidate = widget_invalidate_default,
                          .on_event = widget_on_event_default,
                          .on_paint_self = widget_on_paint_self_default,
                          .on_paint_children = widget_on_paint_children_default,
                          .on_keydown = widget_on_keydown_default,
                          .on_keyup = widget_on_keyup_default,
                          .on_pointer_down = widget_on_pointer_down_default,
                          .on_pointer_move = widget_on_pointer_move_default,
                          .on_pointer_up = widget_on_pointer_up_default,
                          .get_prop = widget_get_prop_default,
                          .set_prop = widget_set_prop_default,
                          .find_target = widget_find_target_default,
                          .on_destroy = widget_on_destroy_default};
