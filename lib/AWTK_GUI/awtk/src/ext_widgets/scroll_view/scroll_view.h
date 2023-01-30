﻿/**
 * File:   scroll_view.h
 * Author: AWTK Develop Team
 * Brief:  scroll_view
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
 * 2018-07-03 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_SCROLL_VIEW_H
#define TK_SCROLL_VIEW_H

#include "../../base/widget.h"
#include "../../base/velocity.h"
#include "../../base/widget_animator.h"

BEGIN_C_DECLS

typedef ret_t (*scroll_view_fix_end_offset_t)(widget_t *widget);
typedef ret_t (*scroll_view_on_scroll_t)(widget_t *widget, int32_t xoffset, int32_t yoffset);
typedef ret_t (*scroll_view_on_scroll_to_t)(widget_t *widget, int32_t xoffset_end,
                                            int32_t yoffset_end, int32_t duration);

/**
 * @class scroll_view_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 滚动视图。
 *
 * scroll\_view\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于scroll\_view\_t控件。
 *
 * 在xml中使用"scroll\_view"标签创建滚动视图控件。如：
 *
 * ```xml
 * <list_view x="0"  y="30" w="100%" h="-80" item_height="60">
 *   <scroll_view name="view" x="0"  y="0" w="100%" h="100%">
 *     <list_item style="odd" children_layout="default(rows=1,cols=0)">
 *       <image draw_type="icon" w="30" image="earth"/>
 *       <label w="-30" text="1.Hello AWTK !">
 *         <switch x="r:10" y="m" w="60" h="20"/>
 *       </label>
 *     </list_item>
 *     ...
 *   </scroll_view>
 *  </list_view>
 * ```
 *
 * > 滚动视图一般作为列表视图的子控件使用。
 *
 * > 更多用法请参考：[list\_view\_m.xml](
 *https://github.com/zlgopen/awtk/blob/master/design/default/ui/list_view_m.xml)
 *
 * 在c代码中使用函数scroll\_view\_create创建列表视图控件。如：
 *
 * ```c
 *  widget_t* scroll_view = scroll_view_create(win, 0, 0, 0, 0);
 * ```
 *
 * 可用通过style来设置控件的显示风格，如背景颜色和边框颜色等(一般情况不需要)。
 *
 */
typedef struct _scroll_view_t
{
  widget_t widget;

  /**
   * @property {wh_t} virtual_w
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 虚拟宽度。
   */
  wh_t virtual_w;
  /**
   * @property {wh_t} virtual_h
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 虚拟高度。
   */
  wh_t virtual_h;
  /**
   * @property {int32_t} xoffset
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * x偏移量。
   */
  int32_t xoffset;
  /**
   * @property {int32_t} yoffset
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * y偏移量。
   */
  int32_t yoffset;
  /**
   * @property {float_t} xspeed_scale
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * x偏移速度比例。
   */
  float_t xspeed_scale;
  /**
   * @property {float_t} yspeed_scale
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * y偏移速度比例。
   */
  float_t yspeed_scale;
  /**
   * @property {bool_t} xslidable
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否允许x方向滑动。
   */
  bool_t xslidable;
  /**
   * @property {bool_t} yslidable
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否允许y方向滑动。
   */
  bool_t yslidable;
  /**
   * @property {bool_t} snap_to_page
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 滚动时offset是否按页面对齐。
   */
  bool_t snap_to_page;
  /**
   * @property {bool_t} move_to_page
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否每次翻一页（当 move_to_page 为ture 的时候才有效果，主要用于区分一次翻一页还是一次翻多页）。
   */
  bool_t move_to_page;
  /**
   * @property {bool_t} recursive
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否递归查找全部子控件。
   */
  bool_t recursive;
  /**
   * @property {float_t} slide_limit_ratio
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 滑动到极限时可继续滑动区域的占比。
   */
  float_t slide_limit_ratio;

  /*private*/
  point_t down;
  bool_t pressed;
  bool_t dragged;
  int32_t xoffset_end;
  int32_t yoffset_end;
  int32_t xoffset_save;
  int32_t yoffset_save;

  int32_t curr_page;
  uint32_t max_page;

  velocity_t velocity;
  widget_animator_t *wa;
  scroll_view_fix_end_offset_t fix_end_offset;
  widget_on_layout_children_t on_layout_children;
  widget_on_paint_children_t on_paint_children;
  widget_on_add_child_t on_add_child;
  widget_find_target_t find_target;
  scroll_view_on_scroll_t on_scroll;
  scroll_view_on_scroll_to_t on_scroll_to;
  bool_t first_move_after_down;
} scroll_view_t;

/**
 * @event {event_t} EVT_SCROLL_START
 * 开始滚动事件。
 */

/**
 * @event {event_t} EVT_SCROLL_END
 * 结束滚动事件。
 */

/**
 * @event {event_t} EVT_SCROLL
 * 滚动事件。
 */

/**
 * @event {event_t} EVT_PAGE_CHANGED
 * 页面改变事件。
 */

/**
 * @event {event_t} EVT_PAGE_CHANGING
 * 页面正在改变。
 */

/**
 * @method scroll_view_create
 * 创建scroll_view对象
 * @annotation ["constructor", "scriptable"]
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} 对象。
 */
widget_t *scroll_view_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method scroll_view_cast
 * 转换为scroll_view对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget scroll_view对象。
 *
 * @return {widget_t*} scroll_view对象。
 */
widget_t *scroll_view_cast(widget_t *widget);

/**
 * @method scroll_view_set_virtual_w
 * 设置虚拟宽度。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {wh_t} w 虚拟宽度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_virtual_w(widget_t *widget, wh_t w);

/**
 * @method scroll_view_set_virtual_h
 * 设置虚拟高度。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {wh_t} h 虚拟高度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_virtual_h(widget_t *widget, wh_t h);

/**
 * @method scroll_view_set_xslidable
 * 设置是否允许x方向滑动。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} xslidable 是否允许滑动。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_xslidable(widget_t *widget, bool_t xslidable);

/**
 * @method scroll_view_set_yslidable
 * 设置是否允许y方向滑动。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} yslidable 是否允许滑动。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_yslidable(widget_t *widget, bool_t yslidable);

/**
 * @method scroll_view_set_snap_to_page
 * 设置滚动时offset是否按页面对齐。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} snap_to_page 是否按页面对齐。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_snap_to_page(widget_t *widget, bool_t snap_to_page);

/**
 * @method scroll_view_set_move_to_page
 * 设置滚动时是否每次翻一页
 * 备注：当 snap_to_page 为ture 的时候才有效果，主要用于区分一次翻一页还是一次翻多页。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} move_to_page 是否每次翻一页。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_move_to_page(widget_t *widget, bool_t move_to_page);

/**
 * @method scroll_view_set_recursive
 * 设置是否递归查找全部子控件。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} recursive 是否递归查找全部子控件。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_recursive(widget_t *widget, bool_t recursive);

/**
 * @method scroll_view_set_recursive_only
 * 设置是否递归查找全部子控件。(不触发repaint和relayout)。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} recursive 是否递归查找全部子控件。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_recursive_only(widget_t *widget, bool_t recursive);

/**
 * @method scroll_view_set_offset
 * 设置偏移量。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {int32_t} xoffset x偏移量。
 * @param {int32_t} yoffset y偏移量。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_offset(widget_t *widget, int32_t xoffset, int32_t yoffset);

/**
 * @method scroll_view_set_speed_scale
 * 设置偏移速度比例。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float_t} xspeed_scale x偏移速度比例。
 * @param {float_t} yspeed_scale y偏移速度比例。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_speed_scale(widget_t *widget, float_t xspeed_scale, float_t yspeed_scale);

/**
 * @method scroll_view_set_slide_limit_ratio
 * 设置滑动到极限时可继续滑动区域的占比。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float_t} slide_limit_ratio 滑动到极限时可继续滑动区域的占比。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_set_slide_limit_ratio(widget_t *widget, float_t slide_limit_ratio);

/**
 * @method scroll_view_scroll_to
 * 滚动到指定的偏移量。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {int32_t} xoffset_end x偏移量。
 * @param {int32_t} yoffset_end y偏移量。
 * @param {int32_t} duration 时间。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_scroll_to(widget_t *widget, int32_t xoffset_end, int32_t yoffset_end,
                            int32_t duration);

/**
 * @method scroll_view_scroll_delta_to
 * 滚动到指定的偏移量。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {int32_t} xoffset_delta x偏移量。
 * @param {int32_t} yoffset_delta y偏移量。
 * @param {int32_t} duration 时间。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t scroll_view_scroll_delta_to(widget_t *widget, int32_t xoffset_delta, int32_t yoffset_delta,
                                  int32_t duration);

#define SCROLL_VIEW(widget) ((scroll_view_t *)(scroll_view_cast(WIDGET(widget))))

#define SCROLL_VIEW_RECURSIVE "recursive"
#define SCROLL_VIEW_SNAP_TO_PAGE "snap_to_page"
#define SCROLL_VIEW_MOVE_TO_PAGE "move_to_page"
#define SCROLL_VIEW_X_SPEED_SCALE "xspeed_scale"
#define SCROLL_VIEW_Y_SPEED_SCALE "yspeed_scale"
#define SCROLL_VIEW_SLIDE_LIMIT_RATIO "slide_limit_ratio"

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(scroll_view);

END_C_DECLS

#endif /*TK_SCROLL_VIEW_H*/
