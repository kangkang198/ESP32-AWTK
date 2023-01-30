﻿/**
 * File:   slide_indicator.h
 * Author: AWTK Develop Team
 * Brief:  slide_indicator
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
 * 2019-05-29 Xu ChaoZe <xuchaoze@zlg.cn> created
 *
 */

#include "../../tkc/mem.h"
#include "../../tkc/utils.h"
#include "../../base/timer.h"
#include "../../base/layout.h"
#include "../../base/enums.h"
#include "../../tkc/easing.h"
#include "../../tkc/utils.h"
#include "../../tkc/func_call_parser.h"
#include "slide_indicator.h"
#include "../../widget_animators/widget_animator_opacity.h"
#include "../../base/widget_animator_manager.h"

#ifndef SLIDE_INDICATOR_HIDE_ANIMATOR_TIME
#define SLIDE_INDICATOR_HIDE_ANIMATOR_TIME 1000
#endif

static ret_t slide_indicator_reset_indicated_widget(widget_t *widget);
static ret_t slide_indicator_reset_indicated_widget(widget_t *widget);
static ret_t slide_indicator_set_indicated_widget(widget_t *widget, widget_t *target);
static ret_t slide_indicator_set_value_impl(widget_t *widget, uint32_t value, bool_t is_on_event);

#define _RADIAN(cx, cy, x, y) atan2(-y + (cy), x - (cx))
#define _DISTANCE(x1, y1, x2, y2) sqrt((x1 - (x2)) * (x1 - (x2)) + (y1 - (y2)) * (y1 - (y2)))

static const key_type_value_t indicator_default_paint_value[] = {
    {"auto", 0, INDICATOR_DEFAULT_PAINT_AUTO},
    {"stroke_dot", 0, INDICATOR_DEFAULT_PAINT_STROKE_DOT},
    {"fill_dot", 0, INDICATOR_DEFAULT_PAINT_FILL_DOT},
    {"stroke_rect", 0, INDICATOR_DEFAULT_PAINT_STROKE_RECT},
    {"fill_rect", 0, INDICATOR_DEFAULT_PAINT_FILL_RECT}};

const key_type_value_t *indicator_default_paint_find(const char *name)
{
  return find_item(indicator_default_paint_value, ARRAY_SIZE(indicator_default_paint_value), name);
}

static pointf_t slide_indicator_eval_anchor(widget_t *widget)
{
  point_t p = {0, 0};
  pointf_t anchor = {0, 0};
  const char *anchor_x = NULL;
  const char *anchor_y = NULL;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, anchor);

  anchor_x = slide_indicator->anchor_x;
  anchor_y = slide_indicator->anchor_y;

  if (TK_STR_IS_EMPTY(anchor_x) || TK_STR_IS_EMPTY(anchor_y))
  {
    widget_t *target = slide_indicator->indicated_widget;
    return_value_if_fail(target != NULL, anchor);

    widget_to_screen(target, &p);
    p.x += target->w / 2;
    p.y += target->h / 2;
    widget_to_local(widget, &p);
  }

  if (TK_STR_IS_EMPTY(anchor_x))
  {
    anchor.x = p.x;
  }
  else
  {
    anchor.x = tk_eval_ratio_or_px(anchor_x, widget->w);
  }

  if (TK_STR_IS_EMPTY(anchor_y))
  {
    anchor.y = p.y;
  }
  else
  {
    anchor.y = tk_eval_ratio_or_px(anchor_y, widget->h);
  }

  return anchor;
}

ret_t slide_indicator_set_anchor_x(widget_t *widget, const char *anchor)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL && anchor != NULL, RET_BAD_PARAMS);

  slide_indicator->anchor_x = tk_str_copy(slide_indicator->anchor_x, anchor);

  return RET_OK;
}

ret_t slide_indicator_set_anchor_y(widget_t *widget, const char *anchor)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL && anchor != NULL, RET_BAD_PARAMS);

  slide_indicator->anchor_y = tk_str_copy(slide_indicator->anchor_y, anchor);

  return RET_OK;
}

static ret_t slide_indicator_get_prop(widget_t *widget, const char *name, value_t *v)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_VALUE) || tk_str_eq(name, WIDGET_PROP_CURR_PAGE))
  {
    value_set_int(v, slide_indicator->value);
    return RET_OK;
  }
  else if (tk_str_eq(name, WIDGET_PROP_MAX))
  {
    value_set_uint32(v, slide_indicator->max);
    return RET_OK;
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_DEFAULT_PAINT))
  {
    value_set_int(v, slide_indicator->default_paint);
    return RET_OK;
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_AUTO_HIDE))
  {
    value_set_int(v, slide_indicator->auto_hide);
    return RET_OK;
  }
  else if (tk_str_eq(name, WIDGET_PROP_MARGIN))
  {
    value_set_int(v, slide_indicator->margin);
    return RET_OK;
  }
  else if (tk_str_eq(name, WIDGET_PROP_SPACING))
  {
    value_set_float(v, slide_indicator->spacing);
    return RET_OK;
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_SIZE))
  {
    value_set_uint32(v, slide_indicator->size);
    return RET_OK;
  }
  else if (tk_str_eq(name, WIDGET_PROP_ANCHOR_X))
  {
    value_set_str(v, slide_indicator->anchor_x);
    return RET_OK;
  }
  else if (tk_str_eq(name, WIDGET_PROP_ANCHOR_Y))
  {
    value_set_str(v, slide_indicator->anchor_y);
    return RET_OK;
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_INDICATED_TARGET))
  {
    value_set_str(v, slide_indicator->indicated_target);
    return RET_OK;
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_transition))
  {
    value_set_bool(v, slide_indicator->transition);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t slide_indicator_set_prop(widget_t *widget, const char *name, const value_t *v)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_VALUE) || tk_str_eq(name, WIDGET_PROP_CURR_PAGE))
  {
    return slide_indicator_set_value(widget, value_int(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_MAX))
  {
    return slide_indicator_set_max(widget, value_uint32(v));
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_DEFAULT_PAINT))
  {
    if (v->type == VALUE_TYPE_STRING)
    {
      const key_type_value_t *kv = indicator_default_paint_find(value_str(v));
      if (kv != NULL)
      {
        slide_indicator_set_default_paint(widget, (indicator_default_paint_t)(kv->value));
      }
    }
    else
    {
      slide_indicator_set_default_paint(widget, (indicator_default_paint_t)value_int(v));
    }
    return RET_OK;
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_AUTO_HIDE))
  {
    return slide_indicator_set_auto_hide(widget, value_uint16(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_MARGIN))
  {
    return slide_indicator_set_margin(widget, value_int(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_SPACING))
  {
    return slide_indicator_set_spacing(widget, value_float(v));
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_SIZE))
  {
    return slide_indicator_set_size(widget, value_uint32(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_ANCHOR_X))
  {
    return slide_indicator_set_anchor_x(widget, value_str(v));
  }
  else if (tk_str_eq(name, WIDGET_PROP_ANCHOR_Y))
  {
    return slide_indicator_set_anchor_y(widget, value_str(v));
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_INDICATED_TARGET))
  {
    return slide_indicator_set_indicated_target(widget, value_str(v));
  }
  else if (tk_str_eq(name, SLIDE_INDICATOR_PROP_transition))
  {
    return slide_indicator_set_transition(widget, value_bool(v));
  }

  return RET_NOT_FOUND;
}

static widget_t *slide_indicator_find_target(widget_t *widget)
{
  value_t v;
  widget_t *parent = NULL;
  return_value_if_fail(widget != NULL, NULL);
  parent = widget->parent;

  WIDGET_FOR_EACH_CHILD_BEGIN(parent, iter, i)
  if (iter != NULL)
  {
    if (widget_get_prop(iter, WIDGET_PROP_CURR_PAGE, &v) == RET_OK &&
        widget_get_prop(iter, WIDGET_PROP_PAGE_MAX_NUMBER, &v) == RET_OK)
    {
      return iter;
    }
  }
  WIDGET_FOR_EACH_CHILD_END()
  return NULL;
}

static ret_t slide_indicator_on_layout_children(widget_t *widget)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  if (slide_indicator->indicated_widget != NULL &&
      slide_indicator->indicated_widget->parent == widget->parent)
  {
  }
  else
  {
    if (slide_indicator_reset_indicated_widget(widget) == RET_OK)
    {
      widget_t *target = NULL;
      if (slide_indicator->indicated_target != NULL)
      {
        target = widget_lookup(widget->parent, slide_indicator->indicated_target, FALSE);
      }
      else
      {
        target = slide_indicator_find_target(widget);
      }
      if (target)
      {
        slide_indicator_set_indicated_widget(widget, target);
      }
    }
  }
  slide_indicator->reset_icon_rect_list = TRUE;
  return widget_layout_children_default(widget);
}

static ret_t slide_indicator_fill_rect(canvas_t *c, rect_t *r, color_t color)
{
  canvas_set_fill_color(c, color);
  canvas_fill_rect(c, r->x, r->y, r->w, r->h);
  return RET_OK;
}

static ret_t slide_indicator_stroke_rect(canvas_t *c, rect_t *r, color_t color)
{
  canvas_set_stroke_color(c, color);
  canvas_stroke_rect(c, r->x, r->y, r->w, r->h);
  return RET_OK;
}

static ret_t slide_indicator_draw_rect_by_vg(canvas_t *c, rectf_t *r, color_t color, bool_t fill)
{
  vgcanvas_t *vg = canvas_get_vgcanvas(c);
  vgcanvas_save(vg);
  vgcanvas_set_global_alpha(vg, c->global_alpha / 255.0f);
  vgcanvas_translate(vg, c->ox, c->oy);
  vgcanvas_begin_path(vg);
  vgcanvas_set_fill_color(vg, color);
  vgcanvas_rect(vg, r->x, r->y, r->w, r->h);
  if (fill)
  {
    vgcanvas_fill(vg);
  }
  else
  {
    vgcanvas_stroke(vg);
  }
  vgcanvas_restore(vg);
  return RET_OK;
}

static ret_t slide_indicator_fill_dot(canvas_t *c, rectf_t *r, color_t color)
{
  float_t radius = tk_min(r->w, r->h) / 2;
  vgcanvas_t *vg = canvas_get_vgcanvas(c);
  vgcanvas_save(vg);
  vgcanvas_set_global_alpha(vg, c->global_alpha / 255.0f);
  vgcanvas_translate(vg, c->ox, c->oy);
  vgcanvas_begin_path(vg);
  vgcanvas_set_fill_color(vg, color);
  vgcanvas_arc(vg, r->x + radius, r->y + radius, radius, 0, M_PI * 2, FALSE);
  vgcanvas_fill(vg);
  vgcanvas_restore(vg);
  return RET_OK;
}

static ret_t slide_indicator_stroke_dot(canvas_t *c, rectf_t *r, color_t color)
{
  float_t radius = tk_min(r->w, r->h) / 2;
  vgcanvas_t *vg = canvas_get_vgcanvas(c);
  vgcanvas_save(vg);
  vgcanvas_set_global_alpha(vg, c->global_alpha / 255.0f);
  vgcanvas_translate(vg, c->ox, c->oy);
  vgcanvas_begin_path(vg);
  vgcanvas_set_stroke_color(vg, color);
  vgcanvas_arc(vg, r->x + radius, r->y + radius, radius, 0, M_PI * 2, FALSE);
  vgcanvas_stroke(vg);
  vgcanvas_restore(vg);
  return RET_OK;
}

static ret_t slide_indicator_default_paint_indicator(widget_t *widget, canvas_t *c, rectf_t *r,
                                                     color_t color)
{
  rect_t rect = rect_from_rectf(r);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  switch (slide_indicator->default_paint)
  {
  case INDICATOR_DEFAULT_PAINT_STROKE_DOT:
    slide_indicator_stroke_dot(c, r, color);
    break;
  case INDICATOR_DEFAULT_PAINT_STROKE_RECT:
    slide_indicator_stroke_rect(c, &rect, color);
    break;
  case INDICATOR_DEFAULT_PAINT_FILL_RECT:
    slide_indicator_fill_rect(c, &rect, color);
    break;
  default:
    slide_indicator_fill_dot(c, r, color);
  }

  return RET_OK;
}

static float_t slide_indicator_get_max_transition_size(slide_indicator_t *slide_indicator)
{
  return slide_indicator->icon_rect_spacing;
}

static ret_t slide_indicator_default_paint_active_indicator(widget_t *widget, canvas_t *c,
                                                            rectf_t *r, color_t color)
{
  rect_t rect;
  rectf_t rectf;
  float_t x, y, w, h;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);
  if (slide_indicator->transition)
  {
    float_t max_transition_size = slide_indicator_get_max_transition_size(slide_indicator);
    w = r->w + max_transition_size;
    h = r->h + max_transition_size;
    x = r->x - max_transition_size / 2;
    y = r->y - max_transition_size / 2;
  }
  else
  {
    x = r->x;
    y = r->y;
    w = r->w;
    h = r->h;
  }
  switch (slide_indicator->default_paint)
  {
  case INDICATOR_DEFAULT_PAINT_STROKE_RECT:
  case INDICATOR_DEFAULT_PAINT_FILL_RECT:
    if (slide_indicator->transition)
    {
      rect = rect_init(x, y, w, h);
      slide_indicator_fill_rect(c, &rect, color);
    }
    else
    {
      rectf = rectf_init(x, y, w, h);
      slide_indicator_draw_rect_by_vg(c, &rectf, color, TRUE);
    }
    break;
  default:
    rectf = rectf_init(x, y, w, h);
    slide_indicator_fill_dot(c, &rectf, color);
  }

  return RET_OK;
}

static ret_t slide_indicator_default_paint_indicator_by_transition(widget_t *widget, canvas_t *c,
                                                                   rectf_t *r, color_t color,
                                                                   color_t selected_color,
                                                                   float_t active_offset)
{
  rectf_t new_rect;
  float_t x, y, w, h, transition_size;
  color_t new_color = color_init(0, 0, 0, 0);
  color_t new_color1 = color_init(0, 0, 0, 0);
  int32_t dr = selected_color.rgba.r - color.rgba.r;
  int32_t dg = selected_color.rgba.g - color.rgba.g;
  int32_t db = selected_color.rgba.b - color.rgba.b;
  int32_t da = selected_color.rgba.a - color.rgba.a;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  new_color.rgba.r = color.rgba.r + dr * active_offset;
  new_color.rgba.g = color.rgba.g + dg * active_offset;
  new_color.rgba.b = color.rgba.b + db * active_offset;
  new_color.rgba.a = color.rgba.a + da * active_offset;
  if (slide_indicator->default_paint == INDICATOR_DEFAULT_PAINT_STROKE_DOT ||
      slide_indicator->default_paint == INDICATOR_DEFAULT_PAINT_STROKE_RECT)
  {
    new_color1.rgba.r = selected_color.rgba.r * active_offset;
    new_color1.rgba.g = selected_color.rgba.g * active_offset;
    new_color1.rgba.b = selected_color.rgba.b * active_offset;
    new_color1.rgba.a = selected_color.rgba.a * active_offset;
  }

  transition_size = slide_indicator_get_max_transition_size(slide_indicator) * active_offset;
  w = r->w + transition_size;
  h = r->h + transition_size;
  x = r->x - transition_size / 2;
  y = r->y - transition_size / 2;

  new_rect = rectf_init(x, y, w, h);
  switch (slide_indicator->default_paint)
  {
  case INDICATOR_DEFAULT_PAINT_STROKE_DOT:
    slide_indicator_stroke_dot(c, &new_rect, new_color);
    slide_indicator_fill_dot(c, &new_rect, new_color1);
    break;
  case INDICATOR_DEFAULT_PAINT_STROKE_RECT:
    slide_indicator_draw_rect_by_vg(c, &new_rect, new_color, FALSE);
    slide_indicator_draw_rect_by_vg(c, &new_rect, new_color1, TRUE);
    break;
  case INDICATOR_DEFAULT_PAINT_FILL_RECT:
    slide_indicator_draw_rect_by_vg(c, &new_rect, new_color, TRUE);
    break;
  default:
    slide_indicator_fill_dot(c, &new_rect, new_color);
    break;
  }

  return RET_OK;
}

static ret_t slide_indicator_paint_one(widget_t *widget, canvas_t *c, rectf_t *r,
                                       float_t active_offset)
{
  bitmap_t img;
  style_t *style = widget->astyle;
  color_t trans = color_init(0, 0, 0, 0);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  color_t color = style_get_color(style, STYLE_ID_FG_COLOR, trans);
  color_t selected_color = style_get_color(style, STYLE_ID_SELECTED_FG_COLOR, trans);
  const char *icon =
      style_get_str(style, active_offset == 1.0f ? STYLE_ID_ACTIVE_ICON : STYLE_ID_ICON, NULL);

  if (!slide_indicator->transition && icon && widget_load_image(widget, icon, &img) == RET_OK)
  {
    int32_t x = r->x + ((int32_t)r->w >> 1);
    int32_t y = r->y + ((int32_t)r->h >> 1);
    canvas_draw_icon(c, &img, x, y);
  }
  else
  {
    if (slide_indicator->transition && 0.0f < active_offset && active_offset < 1.0f)
    {
      slide_indicator_default_paint_indicator_by_transition(widget, c, r, color, selected_color,
                                                            active_offset);
    }
    else if (active_offset == 1.0f)
    {
      slide_indicator_default_paint_active_indicator(widget, c, r, selected_color);
    }
    else
    {
      slide_indicator_default_paint_indicator(widget, c, r, color);
    }
  }

  return RET_OK;
}

static float_t slide_indicator_get_offset_by_index(slide_indicator_t *slide_indicator,
                                                   int32_t index)
{
  int32_t value = slide_indicator->curr_value;
  float_t value_offset = slide_indicator->value_offset;
  if (value_offset > 0.0f)
  {
    if (value == index)
    {
      return 1.0f - value_offset;
    }
    else
    {
      int32_t next_value = value + 1;
      if (index == next_value)
      {
        return value_offset;
      }
      else if (next_value >= slide_indicator->max && slide_indicator->loop && index == 0)
      {
        return value_offset;
      }
    }
  }
  else if (value_offset < 0.0f)
  {
    if (value == index)
    {
      return 1.0f + value_offset;
    }
    else
    {
      int32_t next_value = value - 1;
      if (index == next_value)
      {
        return value_offset;
      }
      else if (next_value < 0 && slide_indicator->loop && index == slide_indicator->max - 1)
      {
        return value_offset;
      }
    }
  }
  else
  {
    if (value == index)
    {
      return 1.0f;
    }
  }
  return 0.0f;
}

static ret_t slide_indicator_paint_arc(widget_t *widget, canvas_t *c)
{
  uint32_t i = 0;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  darray_t *icon_rect_list = &(slide_indicator->icon_rect_list);

  if (icon_rect_list->size != slide_indicator->max || slide_indicator->reset_icon_rect_list)
  {
    rectf_t r;
    uint32_t i = 0;
    float_t offset = 0;
    bool_t ccw = FALSE;
    uint32_t nr = slide_indicator->max;
    uint32_t size = slide_indicator->size;
    pointf_t anchor = slide_indicator_eval_anchor(widget);
    float_t cx = anchor.x;
    float_t cy = anchor.y;
    float_t margin = slide_indicator->margin;
    float_t spacing = (slide_indicator->spacing / 180.0f * M_PI);
    float_t center = _RADIAN(cx, cy, widget->w / 2, widget->h / 2);
    float_t radius = _DISTANCE(cx, cy, widget->w / 2, widget->h / 2) - margin - size / 2;

    if (cx > widget->w / 2 || cy < widget->h / 2)
    {
      ccw = TRUE;
    }

    if (nr % 2)
    {
      offset = ((nr / 2) * spacing);
    }
    else
    {
      offset = ((nr / 2) * spacing - spacing / 2);
    }
    offset = ccw ? (center - offset - 3 * M_PI / 2) : (center + offset + M_PI / 2);

    r = rectf_init(0, 0, size, size);
    darray_clear(icon_rect_list);
    for (i = 0; i < nr; i++)
    {
      rectf_t *tmp = TKMEM_ZALLOC(rectf_t);
      if (ccw)
      {
        r.x = cx + radius * sin(offset + i * spacing) - size / 2;
        r.y = cy + radius * cos(-offset - i * spacing) - size / 2;
      }
      else
      {
        r.x = cx + radius * sin(offset - i * spacing) - size / 2;
        r.y = cy + radius * cos(-offset + i * spacing) - size / 2;
      }
      memcpy(tmp, &r, sizeof(rectf_t));
      darray_push(icon_rect_list, tmp);
    }
    slide_indicator->reset_icon_rect_list = FALSE;
    slide_indicator->icon_rect_spacing = sin(spacing / 2) * radius * 2 / 5;
  }

  for (i = 0; i < icon_rect_list->size; i++)
  {
    rectf_t *tmp = (rectf_t *)darray_get(icon_rect_list, i);
    float_t offset = tk_abs(slide_indicator_get_offset_by_index(slide_indicator, i));
    slide_indicator_paint_one(widget, c, tmp, offset);
  }

  return RET_OK;
}

static ret_t slide_indicator_paint_linear(widget_t *widget, canvas_t *c)
{
  uint32_t i = 0;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  darray_t *icon_rect_list = &(slide_indicator->icon_rect_list);

  if (icon_rect_list->size != slide_indicator->max || slide_indicator->reset_icon_rect_list)
  {
    rectf_t r;
    uint32_t i = 0;
    bool_t ccw = FALSE;
    uint32_t offset = 0;
    uint32_t nr = slide_indicator->max;
    uint32_t size = slide_indicator->size;
    pointf_t anchor = slide_indicator_eval_anchor(widget);
    float_t cx = anchor.x;
    float_t cy = anchor.y;
    bool_t vertical = widget->w < widget->h;
    uint32_t margin = slide_indicator->margin;
    uint32_t spacing = slide_indicator->spacing;
    uint32_t half_size = slide_indicator->size >> 1;

    if (cx > widget->w / 2 || cy < widget->h / 2)
    {
      ccw = TRUE;
    }

    if (nr % 2)
    {
      offset = ((nr / 2) * spacing + half_size);
    }
    else
    {
      offset = ((nr / 2) * spacing - spacing / 2 + half_size);
    }

    if (vertical)
    {
      if (ccw)
      {
        r = rectf_init(widget->w - margin - size, cy - offset, size, size);
      }
      else
      {
        r = rectf_init(margin, cy - offset, size, size);
      }
    }
    else
    {
      if (ccw)
      {
        r = rectf_init(cx - offset, margin, size, size);
      }
      else
      {
        r = rectf_init(cx - offset, widget->h - margin - size, size, size);
      }
    }
    darray_clear(icon_rect_list);
    for (i = 0; i < nr; i++)
    {
      rectf_t *tmp = TKMEM_ZALLOC(rectf_t);
      memcpy(tmp, &r, sizeof(rectf_t));
      darray_push(icon_rect_list, tmp);
      if (vertical)
      {
        r.y += spacing;
      }
      else
      {
        r.x += spacing;
      }
    }
    slide_indicator->icon_rect_spacing = spacing / 5;
    slide_indicator->reset_icon_rect_list = FALSE;
  }

  for (i = 0; i < icon_rect_list->size; i++)
  {
    rectf_t *tmp = (rectf_t *)darray_get(icon_rect_list, i);
    float_t offset = tk_abs(slide_indicator_get_offset_by_index(slide_indicator, i));
    slide_indicator_paint_one(widget, c, tmp, offset);
  }

  return RET_OK;
}

static ret_t slide_indicator_on_animate_end(void *ctx, event_t *e)
{
  widget_t *widget = WIDGET(ctx);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(widget != NULL && slide_indicator != NULL, RET_REMOVE);

  slide_indicator->wa_opactiy = NULL;
  widget_set_visible(widget, FALSE, FALSE);
  widget_set_opacity(widget, 0xff);
  return RET_REMOVE;
}

static ret_t slide_indicator_set_visible(widget_t *widget, bool_t visible)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(widget != NULL && slide_indicator != NULL, RET_REMOVE);

  if (slide_indicator->wa_opactiy != NULL)
  {
    widget_animator_destroy(slide_indicator->wa_opactiy);
    slide_indicator->wa_opactiy = NULL;
  }

  if (!visible && slide_indicator->auto_hide)
  {
    slide_indicator->wa_opactiy = widget_animator_opacity_create(
        widget, SLIDE_INDICATOR_HIDE_ANIMATOR_TIME, 0, EASING_SIN_INOUT);
    widget_animator_on(slide_indicator->wa_opactiy, EVT_ANIM_END, slide_indicator_on_animate_end,
                       slide_indicator);
    widget_animator_opacity_set_params(slide_indicator->wa_opactiy, 0xff, 0);
    widget_set_opacity(widget, 0xff);
    widget_animator_start(slide_indicator->wa_opactiy);
  }
  else
  {
    if (visible)
    {
      widget_set_opacity(widget, 0xff);
    }
    widget_set_visible(widget, visible);
  }

  return RET_OK;
}

static ret_t slide_indicator_target_on_value_changing(void *ctx, event_t *e)
{
  int32_t value = 0;
  int32_t offset = 0;
  widget_t *widget = WIDGET(e->target);
  widget_t *indicator = WIDGET(ctx);
  offset_change_event_t *evt = (offset_change_event_t *)e;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(indicator);
  return_value_if_fail(widget != NULL && slide_indicator != NULL && evt != NULL, RET_BAD_PARAMS);
  if (!slide_indicator->transition ||
      (slide_indicator->transition && slide_indicator->is_value_changing))
  {
    slide_indicator->value_offset = 0.0f;
    return RET_OK;
  }

  offset = (int32_t)(evt->new_offset - evt->old_offset);
  if (indicator->w > indicator->h)
  {
    value = offset / widget->w;
    slide_indicator->value_offset = offset / (float_t)widget->w - value;
  }
  else
  {
    value = offset / widget->h;
    slide_indicator->value_offset = offset / (float_t)widget->h - value;
  }
  value = slide_indicator->value + value;
  if (slide_indicator->loop)
  {
    if (value < 0)
    {
      value += slide_indicator->max;
    }
    slide_indicator->curr_value = value % slide_indicator->max;
  }
  else
  {
    if (slide_indicator->value == 0 && offset < 0)
    {
      slide_indicator->curr_value = 0;
      slide_indicator->value_offset = 0.0f;
    }
    else if (slide_indicator->value == slide_indicator->max - 1 && offset > 0)
    {
      slide_indicator->value_offset = 0.0f;
      slide_indicator->curr_value = slide_indicator->max - 1;
    }
    else
    {
      slide_indicator->curr_value = value;
    }
  }

  slide_indicator->loop = widget_get_prop_bool(widget, WIDGET_PROP_LOOP, FALSE);
  widget_invalidate(widget, NULL);

  return RET_OK;
}

static ret_t slide_indicator_target_on_value_changed(void *ctx, event_t *e)
{
  value_t v;
  widget_t *widget = WIDGET(e->target);
  widget_t *indicator = WIDGET(ctx);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(indicator);
  return_value_if_fail(widget != NULL && slide_indicator != NULL, RET_BAD_PARAMS);

  if (widget_get_prop(widget, WIDGET_PROP_CURR_PAGE, &v) != RET_OK)
  {
    widget_get_prop(widget, WIDGET_PROP_VALUE, &v);
  }

  if (slide_indicator->value != value_int(&v))
  {
    slide_indicator_set_value_impl(indicator, value_int(&v), TRUE);
  }

  if (widget_get_prop(widget, WIDGET_PROP_PAGE_MAX_NUMBER, &v) != RET_OK)
  {
    uint32_t max = value_int(&v);
    if (slide_indicator->max != max)
    {
      slide_indicator->max = max;
    }
  }
  slide_indicator->value_offset = 0.0f;
  slide_indicator->is_value_changing = FALSE;
  slide_indicator->curr_value = slide_indicator->value;
  widget_invalidate(widget, NULL);

  return RET_OK;
}

static ret_t slide_indicator_target_on_move_resize(void *ctx, event_t *e)
{
  widget_t *widget = WIDGET(e->target);
  widget_t *indicator = WIDGET(ctx);
  return_value_if_fail(widget != NULL && indicator != NULL, RET_BAD_PARAMS);
  return widget_invalidate_force(indicator, NULL);
}

static ret_t slide_indicator_target_on_pointer_move(void *ctx, event_t *e)
{
  widget_t *indicator = WIDGET(ctx);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(indicator);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  if (slide_indicator->auto_hide)
  {
    pointer_event_t *evt = (pointer_event_t *)e;
    slide_indicator->last_move_point.x = evt->x;
    slide_indicator->last_move_point.y = evt->y;
    slide_indicator->last_move_point_time = timer_manager()->get_time();
    slide_indicator_set_visible(indicator, TRUE);
  }

  return RET_OK;
}

static ret_t slide_indicator_target_on_destroy(void *ctx, event_t *e)
{
  widget_t *indicator = WIDGET(ctx);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(indicator);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator->indicated_widget = NULL;

  return RET_OK;
}

static ret_t slide_indicator_reset_indicated_widget(widget_t *widget)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  if (slide_indicator->indicated_widget != NULL)
  {
    widget_t *target = slide_indicator->indicated_widget;
    widget_off_by_func(target, EVT_MOVE, slide_indicator_target_on_move_resize, widget);
    widget_off_by_func(target, EVT_MOVE_RESIZE, slide_indicator_target_on_move_resize, widget);
    widget_off_by_func(target, EVT_RESIZE, slide_indicator_target_on_move_resize, widget);
    widget_off_by_func(target, EVT_PAGE_CHANGED, slide_indicator_target_on_value_changed, widget);
    widget_off_by_func(target, EVT_PAGE_CHANGING, slide_indicator_target_on_value_changing, widget);
    widget_off_by_func(target, EVT_DESTROY, slide_indicator_target_on_destroy, widget);

    if (slide_indicator->auto_hide)
    {
      widget_off_by_func(target, EVT_POINTER_MOVE, slide_indicator_target_on_pointer_move, widget);
    }
    slide_indicator->indicated_widget = NULL;
  }

  return RET_OK;
}

static ret_t slide_indicator_set_indicated_widget(widget_t *widget, widget_t *target)
{
  value_t v;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL && target != NULL, RET_BAD_PARAMS);

  slide_indicator->indicated_widget = target;

  if (widget_get_prop(target, WIDGET_PROP_PAGE_MAX_NUMBER, &v) == RET_OK)
  {
    slide_indicator->max = value_uint32(&v);
    slide_indicator->chilren_indicated = TRUE;
  }

  if (widget_get_prop(target, WIDGET_PROP_CURR_PAGE, &v) != RET_OK)
  {
    widget_get_prop(target, WIDGET_PROP_VALUE, &v);
  }
  slide_indicator_set_value(widget, value_uint32(&v));

  widget_on(target, EVT_MOVE, slide_indicator_target_on_move_resize, widget);
  widget_on(target, EVT_MOVE_RESIZE, slide_indicator_target_on_move_resize, widget);
  widget_on(target, EVT_RESIZE, slide_indicator_target_on_move_resize, widget);
  widget_on(target, EVT_PAGE_CHANGED, slide_indicator_target_on_value_changed, widget);
  widget_on(target, EVT_PAGE_CHANGING, slide_indicator_target_on_value_changing, widget);
  widget_on(target, EVT_DESTROY, slide_indicator_target_on_destroy, widget);
  if (slide_indicator->auto_hide)
  {
    widget_on(target, EVT_POINTER_MOVE, slide_indicator_target_on_pointer_move, widget);
  }

  return RET_OK;
}

ret_t slide_indicator_on_paint_begin(widget_t *widget, canvas_t *c)
{
  value_t v;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(widget != NULL && slide_indicator != NULL, RET_BAD_PARAMS);

  if (slide_indicator->chilren_indicated)
  {
    if (widget_get_prop(slide_indicator->indicated_widget, WIDGET_PROP_PAGE_MAX_NUMBER, &v) ==
        RET_OK)
    {
      slide_indicator->max = value_uint32(&v);
    }
  }
  return RET_OK;
}

static ret_t slide_indicator_on_paint_self(widget_t *widget, canvas_t *c)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(widget != NULL && slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator_paint_linear(widget, c);
  return RET_OK;
}

static ret_t slide_indicator_arc_on_paint_self(widget_t *widget, canvas_t *c)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(widget != NULL && slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator_paint_arc(widget, c);
  return RET_OK;
}

ret_t slide_indicator_on_destroy(widget_t *widget)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(widget != NULL && slide_indicator != NULL, RET_BAD_PARAMS);

  if (slide_indicator->check_hide_idle != TK_INVALID_ID)
  {
    idle_remove(slide_indicator->check_hide_idle);
  }

  darray_deinit(&(slide_indicator->icon_rect_list));

  slide_indicator_reset_indicated_widget(widget);
  TKMEM_FREE(slide_indicator->indicated_target);
  TKMEM_FREE(slide_indicator->anchor_x);
  TKMEM_FREE(slide_indicator->anchor_y);

  return RET_OK;
}

static ret_t slide_indicator_on_click(widget_t *widget, int32_t x, int32_t y)
{
  uint32_t i = 0;
  point_t p = {x, y};
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL && widget != NULL, RET_BAD_PARAMS);
  widget_to_local(widget, &p);
  for (; i < slide_indicator->icon_rect_list.size; i++)
  {
    rectf_t *r = (rectf_t *)darray_get(&(slide_indicator->icon_rect_list), i);

    if (p.x >= r->x && p.y >= r->y && p.x < r->x + r->w && p.y < r->y + r->h)
    {
      slide_indicator_set_value(widget, i);
      break;
    }
  }
  return RET_OK;
}

static ret_t slide_indicator_on_event(widget_t *widget, event_t *e)
{
  uint16_t type = e->type;
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL && widget != NULL, RET_BAD_PARAMS);

  switch (type)
  {
  case EVT_MOVE:
  case EVT_RESIZE:
  case EVT_MOVE_RESIZE:
    slide_indicator->reset_icon_rect_list = TRUE;
    break;
  case EVT_POINTER_DOWN:
    slide_indicator->pressed = TRUE;
    widget_set_state(widget, WIDGET_STATE_PRESSED);
    break;
  case EVT_POINTER_MOVE:
    slide_indicator_target_on_pointer_move(widget, e);
    break;
  case EVT_POINTER_UP:
    if (slide_indicator->pressed)
    {
      pointer_event_t *evt = (pointer_event_t *)e;
      slide_indicator_on_click(widget, evt->x, evt->y);
    }
  case EVT_POINTER_DOWN_ABORT:
    slide_indicator->pressed = FALSE;
    widget_set_state(widget, WIDGET_STATE_NORMAL);
    break;
  default:
    break;
  }
  return RET_OK;
}

ret_t slide_indicator_on_idle_chech_hide(const idle_info_t *idle)
{
  uint64_t curr_time = timer_manager()->get_time();
  widget_t *widget = WIDGET(idle->ctx);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  if (!slide_indicator->auto_hide)
  {
    slide_indicator->check_hide_idle = TK_INVALID_ID;
    return RET_OK;
  }
  if (slide_indicator->last_move_point_time + slide_indicator->auto_hide < curr_time)
  {
    if (slide_indicator->wa_opactiy == NULL && widget->visible)
    {
      slide_indicator_set_visible(widget, FALSE);
    }
  }

  return RET_REPEAT;
}

static const char *s_slide_indicator_properties[] = {WIDGET_PROP_VALUE,
                                                     WIDGET_PROP_MAX,
                                                     SLIDE_INDICATOR_PROP_DEFAULT_PAINT,
                                                     SLIDE_INDICATOR_PROP_AUTO_HIDE,
                                                     SLIDE_INDICATOR_PROP_INDICATED_TARGET,
                                                     WIDGET_PROP_MARGIN,
                                                     WIDGET_PROP_SPACING,
                                                     SLIDE_INDICATOR_PROP_SIZE,
                                                     WIDGET_PROP_ANCHOR_X,
                                                     WIDGET_PROP_ANCHOR_Y,
                                                     NULL};

TK_DECL_VTABLE(slide_indicator_linear) = {.size = sizeof(slide_indicator_t),
                                          .type = WIDGET_TYPE_SLIDE_INDICATOR,
                                          .clone_properties = s_slide_indicator_properties,
                                          .persistent_properties = s_slide_indicator_properties,
                                          .get_parent_vt = TK_GET_PARENT_VTABLE(widget),
                                          .create = slide_indicator_create,
                                          .on_event = slide_indicator_on_event,
                                          .get_prop = slide_indicator_get_prop,
                                          .set_prop = slide_indicator_set_prop,
                                          .on_layout_children = slide_indicator_on_layout_children,
                                          .on_paint_begin = slide_indicator_on_paint_begin,
                                          .on_paint_self = slide_indicator_on_paint_self,
                                          .on_destroy = slide_indicator_on_destroy};

TK_DECL_VTABLE(slide_indicator_arc) = {.size = sizeof(slide_indicator_t),
                                       .type = WIDGET_TYPE_SLIDE_INDICATOR_ARC,
                                       .clone_properties = s_slide_indicator_properties,
                                       .persistent_properties = s_slide_indicator_properties,
                                       .get_parent_vt = TK_GET_PARENT_VTABLE(widget),
                                       .create = slide_indicator_create_arc,
                                       .on_event = slide_indicator_on_event,
                                       .get_prop = slide_indicator_get_prop,
                                       .set_prop = slide_indicator_set_prop,
                                       .on_layout_children = slide_indicator_on_layout_children,
                                       .on_paint_begin = slide_indicator_on_paint_begin,
                                       .on_paint_self = slide_indicator_arc_on_paint_self,
                                       .on_destroy = slide_indicator_on_destroy};

/*create*/

widget_t *slide_indicator_create_internal(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h,
                                          const widget_vtable_t *vt)
{
  widget_t *widget = widget_create(parent, vt, x, y, w, h);
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, NULL);

  slide_indicator->default_paint = INDICATOR_DEFAULT_PAINT_AUTO;
  slide_indicator->auto_hide = 0;
  slide_indicator->margin = 0;
  slide_indicator->spacing = 16;
  slide_indicator->size = 8;
  slide_indicator->wa_opactiy = NULL;
  slide_indicator->chilren_indicated = FALSE;
  slide_indicator->indicated_widget = NULL;
  slide_indicator->indicated_target = NULL;
  slide_indicator->reset_icon_rect_list = FALSE;
  slide_indicator->check_hide_idle = TK_INVALID_ID;

  darray_init(&(slide_indicator->icon_rect_list), 10, default_destroy, NULL);
  return widget;
}

widget_t *slide_indicator_create_linear(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
  return slide_indicator_create_internal(parent, x, y, w, h, TK_REF_VTABLE(slide_indicator_linear));
}

widget_t *slide_indicator_create_arc(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
  widget_t *widget =
      slide_indicator_create_internal(parent, x, y, w, h, TK_REF_VTABLE(slide_indicator_arc));
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, NULL);

  slide_indicator->spacing = 5;
  return widget;
}

widget_t *slide_indicator_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
  return slide_indicator_create_linear(parent, x, y, w, h);
}

ret_t slide_indicator_set_value(widget_t *widget, uint32_t value)
{
  return slide_indicator_set_value_impl(widget, value, FALSE);
}

static ret_t slide_indicator_set_value_impl(widget_t *widget, uint32_t value, bool_t is_on_event)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  if (slide_indicator->value != value)
  {
    value_t v;
    ret_t ret = RET_OK;
    value_change_event_t evt;
    value_change_event_init(&evt, EVT_VALUE_WILL_CHANGE, widget);
    value_set_uint32(&(evt.old_value), slide_indicator->value);
    value_set_uint32(&(evt.new_value), value);

    if (widget_dispatch(widget, (event_t *)&evt) != RET_STOP)
    {
      if (slide_indicator->indicated_widget != NULL && !is_on_event)
      {
        if (widget_get_prop(slide_indicator->indicated_widget, WIDGET_PROP_CURR_PAGE, &v) !=
            RET_OK)
        {
          widget_get_prop(slide_indicator->indicated_widget, WIDGET_PROP_VALUE, &v);
        }
        if (value != value_int(&v))
        {
          value_t v1;
          value_set_int(&v1, value);
          ret = widget_set_prop(slide_indicator->indicated_widget, WIDGET_PROP_CURR_PAGE, &v1);
        }
        else
        {
          ret = RET_FAIL;
        }
        slide_indicator->is_value_changing = TRUE;
      }

      if (ret == RET_OK)
      {
        slide_indicator->value = value;
        evt.e.type = EVT_VALUE_CHANGED;
        widget_dispatch(widget, (event_t *)&evt);
        widget_dispatch_simple_event(widget, EVT_PAGE_CHANGED);
        widget_invalidate(widget, NULL);
      }
    }
  }
  slide_indicator->curr_value = slide_indicator->value;
  return RET_OK;
}

ret_t slide_indicator_set_max(widget_t *widget, uint32_t max)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator->max = max;
  slide_indicator->chilren_indicated = FALSE;
  slide_indicator->reset_icon_rect_list = TRUE;
  return widget_invalidate(widget, NULL);
}

ret_t slide_indicator_set_default_paint(widget_t *widget, indicator_default_paint_t default_paint)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator->default_paint = default_paint;
  widget_invalidate(widget, NULL);

  return RET_OK;
}

ret_t slide_indicator_set_auto_hide(widget_t *widget, uint16_t auto_hide)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator->auto_hide = auto_hide;
  if (auto_hide)
  {
    if (slide_indicator->check_hide_idle != TK_INVALID_ID)
    {
      idle_remove(slide_indicator->check_hide_idle);
    }
    slide_indicator->last_move_point_time = timer_manager()->get_time();
    slide_indicator->check_hide_idle = idle_add(slide_indicator_on_idle_chech_hide, widget);
  }
  else
  {
    if (slide_indicator->check_hide_idle != TK_INVALID_ID)
    {
      idle_remove(slide_indicator->check_hide_idle);
    }
  }
  return RET_OK;
}

ret_t slide_indicator_set_margin(widget_t *widget, int32_t margin)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator->margin = margin;
  slide_indicator->reset_icon_rect_list = TRUE;
  return widget_invalidate(widget, NULL);
}

ret_t slide_indicator_set_spacing(widget_t *widget, float_t spacing)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator->spacing = spacing;
  slide_indicator->reset_icon_rect_list = TRUE;
  return widget_invalidate(widget, NULL);
}

ret_t slide_indicator_set_size(widget_t *widget, uint32_t size)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator->size = size;
  slide_indicator->reset_icon_rect_list = TRUE;
  return widget_invalidate(widget, NULL);
}

ret_t slide_indicator_set_anchor(widget_t *widget, const char *anchor_x, const char *anchor_y)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator_set_anchor_x(widget, anchor_x);
  slide_indicator_set_anchor_y(widget, anchor_y);
  slide_indicator->reset_icon_rect_list = TRUE;
  return widget_invalidate(widget, NULL);
}

ret_t slide_indicator_set_indicated_target(widget_t *widget, const char *target_name)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);

  slide_indicator_reset_indicated_widget(widget);
  TKMEM_FREE(slide_indicator->indicated_target);

  slide_indicator->indicated_target = tk_str_copy(slide_indicator->indicated_target, target_name);

  return RET_OK;
}

ret_t slide_indicator_set_transition(widget_t *widget, bool_t transition)
{
  slide_indicator_t *slide_indicator = SLIDE_INDICATOR(widget);
  return_value_if_fail(slide_indicator != NULL, RET_BAD_PARAMS);
  slide_indicator->transition = transition;
  return RET_OK;
}

widget_t *slide_indicator_cast(widget_t *widget)
{
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, slide_indicator_linear) ||
                           WIDGET_IS_INSTANCE_OF(widget, slide_indicator_arc),
                       NULL);

  return widget;
}
