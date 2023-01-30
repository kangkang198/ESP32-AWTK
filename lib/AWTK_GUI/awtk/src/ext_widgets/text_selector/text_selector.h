﻿/**
 * File:   text_selector.h
 * Author: AWTK Develop Team
 * Brief:  text_selector
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
 * 2018-09-25 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TEXT_SELECTOR_H
#define TK_TEXT_SELECTOR_H

#include "../../base/widget.h"
#include "../../base/velocity.h"
#include "../../base/widget_animator.h"

BEGIN_C_DECLS

typedef struct _text_selector_option_t
{
  int32_t value;
  struct _text_selector_option_t *next;
  char *tr_text;
  wstr_t text;
} text_selector_option_t;

/**
 * @class text_selector_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 文本选择器控件，通常用于选择日期和时间等。
 *
 * > XXX: 目前需要先设置options和visible_nr，再设置其它参数(在XML中也需要按此顺序)。
 *
 * text\_selector\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于text\_selector\_t控件。
 *
 * 在xml中使用"text\_selector"标签创建文本选择器控件。如：
 *
 * ```xml
 * <text_selector options="red;green;blue;gold;orange" visible_nr="3" text="red"/>
 * ```
 *
 * > 更多用法请参考：[text\_selector.xml](
 * https://github.com/zlgopen/awtk/blob/master/design/default/ui/text_selector.xml)
 *
 * 在c代码中使用函数text\_selector\_create创建文本选择器控件。如：
 *
 * ```c
 * widget_t* ts = text_selector_create(win, 10, 10, 80, 150);
 * text_selector_set_options(ts, "1:red;2:green;3:blue;4:orange;5:gold");
 * text_selector_set_value(ts, 1);
 * widget_use_style(ts, "dark");
 * ```
 *
 * > 完整示例请参考：[text\_selector demo](
 * https://github.com/zlgopen/awtk-c-demos/blob/master/demos/text_selector.c)
 *
 * 可用通过style来设置控件的显示风格，如字体和背景颜色等。如：
 *
 * ```xml
 * <style name="dark" fg_color="#a0a0a0"  text_color="black" text_align_h="center">
 *   <normal     bg_color="#ffffff" mask_color="#404040" border_color="#404040"/>
 * </style>
 * ```
 *
 * > 更多用法请参考：[theme default](
 * https://github.com/zlgopen/awtk/blob/master/design/default/styles/default.xml#L443)
 *
 */
typedef struct _text_selector_t
{
  widget_t widget;

  /**
   * @property {uint32_t} visible_nr
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 可见的选项数量(只能是1或者3或者5，缺省为5)。
   */
  uint32_t visible_nr;

  /**
   * @property {int32_t} selected_index
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 当前选中的选项。
   */
  int32_t selected_index;

  /**
   * @property {char*} options
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 设置可选项(英文冒号(:)分隔值和文本，英文分号(;)分隔选项，如:1:red;2:green;3:blue)。
   * 对于数值选项，也可以指定一个范围，用英文负号(-)分隔起始值、结束值和格式。
   * 如："1-7-%02d"表示1到7，格式为『02d』，格式为可选，缺省为『%d』。
   * > 如果数据本身中有英文冒号(:)、英文分号(;)和英文负号(-)。请用16进制转义。
   * > * 英文冒号(:)写为\\x3a
   * > * 英文冒号(;)写为\\x3b
   * > * 英文冒号(-)写为\\x2d
   */
  char *options;

  /**
   * @property {float_t} yspeed_scale
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * y偏移速度比例。
   */
  float_t yspeed_scale;

  /**
   * @property {uint32_t} animating_time
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 滚动动画播放时间。(单位毫秒)
   */
  uint32_t animating_time;

  /**
   * @property {bool_t} localize_options
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否本地化(翻译)选项(缺省为FALSE)。
   */
  bool_t localize_options;

  /**
   * @property {bool_t} loop_options
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否循环选项(缺省为FALSE)。
   */
  bool_t loop_options;

  /**
   * @property {bool_t} enable_value_animator
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否修改值时启用动画。
   */
  bool_t enable_value_animator;

  /**
   * @property {easing_type_t} mask_easing
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 绘制蒙版的变化趋势。
   */
  easing_type_t mask_easing;

  /**
   * @property {float_t} mask_area_scale
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 绘制蒙版的区域占比（范围0~1）。
   */
  float_t mask_area_scale;

  /*private*/
  bool_t pressed;
  bool_t is_init;
  str_t text;
  int32_t ydown;
  int32_t yoffset;
  int32_t yoffset_save;
  velocity_t velocity;
  widget_animator_t *wa;
  int32_t draw_widget_y;
  int32_t draw_widget_h;
  uint32_t locale_info_id;
  int32_t last_selected_index;
  text_selector_option_t *option_items;
} text_selector_t;

/**
 * @event {value_change_event_t} EVT_VALUE_WILL_CHANGE
 * 值(当前项)即将改变事件。
 */

/**
 * @event {value_change_event_t} EVT_VALUE_CHANGED
 * 值(当前项)改变事件。
 */

/**
 * @method text_selector_create
 * 创建text_selector对象
 * @annotation ["constructor", "scriptable"]
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} 对象。
 */
widget_t *text_selector_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method text_selector_cast
 * 转换text_selector对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget text_selector对象。
 *
 * @return {widget_t*} text_selector对象。
 */
widget_t *text_selector_cast(widget_t *widget);

/**
 * @method text_selector_reset_options
 * 重置所有选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_reset_options(widget_t *widget);

/**
 * @method text_selector_count_options
 * 获取选项个数。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 *
 * @return {int32_t} 返回选项个数。
 */
int32_t text_selector_count_options(widget_t *widget);

/**
 * @method text_selector_append_option
 * 追加一个选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {int32_t} value 值。
 * @param {char*} text 文本。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_append_option(widget_t *widget, int32_t value, const char *text);

/**
 * @method text_selector_set_options
 * 设置选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {char*} options 选项。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_options(widget_t *widget, const char *options);

/**
 * @method text_selector_set_range_options_ex
 * 设置一系列的整数选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {int32_t} start 起始值。
 * @param {uint32_t} nr 个数。
 * @param {int32_t} step 步长。
 * @param {const char*} format 选项的格式化。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_range_options_ex(widget_t *widget, int32_t start, uint32_t nr, int32_t step,
                                         const char *format);

/**
 * @method text_selector_set_range_options
 * 设置一系列的整数选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {int32_t} start 起始值。
 * @param {uint32_t} nr 个数。
 * @param {int32_t} step 步长。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_range_options(widget_t *widget, int32_t start, uint32_t nr, int32_t step);

/**
 * @method text_selector_get_option
 * 获取第index个选项。
 * @param {widget_t*} widget text_selector对象。
 * @param {uint32_t} index 选项的索引。
 *
 * @return {text_selector_option_t*} 成功返回选项，失败返回NULL。
 */
text_selector_option_t *text_selector_get_option(widget_t *widget, uint32_t index);

/**
 * @method text_selector_get_value
 * 获取text_selector的值。
 * @alias text_selector_get_value_int
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 *
 * @return {int32_t} 返回值。
 */
int32_t text_selector_get_value(widget_t *widget);

/**
 * @method text_selector_set_value
 * 设置text_selector的值。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {int32_t} value 值。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_value(widget_t *widget, int32_t value);

/**
 * @method text_selector_get_text
 * 获取text_selector的文本。
 * @annotation ["scriptable"]
 * @alias text_selector_get_text_value
 * @param {widget_t*} widget text_selector对象。
 *
 * @return {const char*} 返回文本。
 */
const char *text_selector_get_text(widget_t *widget);

/**
 * @method text_selector_set_text
 * 设置text_selector的文本。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {const char*} text 文本。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_text(widget_t *widget, const char *text);

/**
 * @method text_selector_set_selected_index
 * 设置第index个选项为当前选中的选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {uint32_t} index 选项的索引。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_selected_index(widget_t *widget, uint32_t index);

/**
 * @method text_selector_set_visible_nr
 * 设置可见的选项数。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {uint32_t} visible_nr 选项数。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_visible_nr(widget_t *widget, uint32_t visible_nr);

/**
 * @method text_selector_set_localize_options
 * 设置是否本地化(翻译)选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {bool_t} localize_options 是否本地化(翻译)选项。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_localize_options(widget_t *widget, bool_t localize_options);

/**
 * @method text_selector_set_loop_options
 * 设置是否循环选项。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget text_selector对象。
 * @param {bool_t} loop_options 是否循环选项。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_loop_options(widget_t *widget, bool_t loop_options);

/**
 * @method text_selector_set_yspeed_scale
 * 设置Y轴偏移速度比例。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float_t} yspeed_scale y偏移速度比例。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_yspeed_scale(widget_t *widget, float_t yspeed_scale);

/**
 * @method text_selector_set_animating_time
 * 设置滚动动画播放时间。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {uint32_t} animating_time 滚动动画播放时间。(单位毫秒)
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_animating_time(widget_t *widget, uint32_t animating_time);

/**
 * @method text_selector_set_enable_value_animator
 * 设置是否修改值时启用动画。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} enable_value_animator 是否修改值时启用动画
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_enable_value_animator(widget_t *widget, bool_t enable_value_animator);

/**
 * @method text_selector_set_mask_easing
 * 设置绘制蒙版的变化趋势。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {easing_type_t} mask_easing 绘制蒙版的变化趋势。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_mask_easing(widget_t *widget, easing_type_t mask_easing);

/**
 * @method text_selector_set_mask_area_scale
 * 设置绘制蒙版的区域占比（范围0~1）。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float_t} mask_area_scale 绘制蒙版的区域占比（范围0~1）。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_selector_set_mask_area_scale(widget_t *widget, float_t mask_area_scale);

#define TEXT_SELECTOR_PROP_VISIBLE_NR "visible_nr"
#define WIDGET_TYPE_TEXT_SELECTOR "text_selector"
#define TEXT_SELECTOR_PROP_LOOP_OPTIONS "loop_options"
#define TEXT_SELECTOR_PROP_Y_SPEED_SCALE "yspeed_scale"
#define TEXT_SELECTOR_PROP_ANIMATION_TIME "animating_time"
#define TEXT_SELECTOR_PROP_ENABLE_VALUE_ANIMATOR "enable_value_animator"
#define TEXT_SELECTOR_PROP_MASH_EASING "mask_easing"
#define TEXT_SELECTOR_PROP_MASH_AREA_SCALE "mask_area_scale"
#define TEXT_SELECTOR(widget) ((text_selector_t *)(text_selector_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(text_selector);

/*public for test*/
ret_t text_selector_parse_options(widget_t *widget, const char *str);

END_C_DECLS

#endif /*TK_TEXT_SELECTOR_H*/
