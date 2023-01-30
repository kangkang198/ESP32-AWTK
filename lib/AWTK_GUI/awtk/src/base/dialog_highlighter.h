﻿/**
 * File:   dialog_highlighter.h
 * Author: AWTK Develop Team
 * Brief:  dialog_highlighter
 *
 * Copyright (c) 2018 - 2022  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied highlighterrranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2019-03-27 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_DIALOG_HIGHLIGHTER_H
#define TK_DIALOG_HIGHLIGHTER_H

#include "widget.h"

BEGIN_C_DECLS

struct _dialog_highlighter_t;
typedef struct _dialog_highlighter_t dialog_highlighter_t;

typedef ret_t (*dialog_highlighter_set_system_bar_alpha_t)(dialog_highlighter_t *h, uint8_t alpha);
typedef uint8_t (*dialog_highlighter_get_alpha_t)(dialog_highlighter_t *h, float_t percent);
typedef ret_t (*dialog_highlighter_draw_mask_t)(dialog_highlighter_t *h, canvas_t *c,
                                                float_t percent);
typedef ret_t (*dialog_highlighter_prepare_t)(dialog_highlighter_t *h, canvas_t *c);
typedef ret_t (*dialog_highlighter_draw_t)(dialog_highlighter_t *h, float_t percent);
typedef bool_t (*dialog_highlighter_is_dynamic_t)(dialog_highlighter_t *h);
typedef ret_t (*dialog_highlighter_on_destroy_t)(dialog_highlighter_t *h);

typedef dialog_highlighter_t *(*dialog_highlighter_create_t)(tk_object_t *args);

typedef struct _dialog_highlighter_vtable_t
{
  const char *type;
  const char *desc;
  uint32_t size;
  dialog_highlighter_draw_t draw;
  dialog_highlighter_prepare_t prepare;
  dialog_highlighter_draw_mask_t draw_mask;
  dialog_highlighter_get_alpha_t get_alpha;
  dialog_highlighter_is_dynamic_t is_dynamic;
  dialog_highlighter_on_destroy_t on_destroy;
  dialog_highlighter_set_system_bar_alpha_t set_system_bar_alpha;
} dialog_highlighter_vtable_t;

/**
 * @enum dialog_highlighter_type_t
 * @prefix DIALOG_HIGHLIGHTER_
 * @type string
 * 内置的对话框高亮策略。
 */

/**
 * @const DIALOG_HIGHLIGHTER_DEFAULT
 * 缺省的对话框高亮策略。
 */
#define DIALOG_HIGHLIGHTER_DEFAULT "default"

/**
 * @class dialog_highlighter_t
 * 对话框高亮策略。
 *
 *> 高亮策略的基本思路是对背景进行处理，比如将背景变暗或变模糊。
 *
 */
struct _dialog_highlighter_t
{
  emitter_t emitter;

  /**
   * @property {bitmap_t} img
   * 底层窗口的截图。
   */
  bitmap_t img;
  /**
   * @property {canvas_t*} canvas
   * 画布。
   */
  canvas_t *canvas;

  /**
   * @property {widget_t*} dialog
   * 对应的对话框。
   */
  widget_t *dialog;

  /**
   * @property {rect_t*} clip_rect
   * 截图的显示裁减区
   */
  rect_t clip_rect;

  /**
   * @property {widget_t*} win
   * 底层窗口。
   */
  widget_t *win;

  /*private*/
  const dialog_highlighter_vtable_t *vt;
};

/**
 * @method dialog_highlighter_create
 * 创建对话框高亮策略对象。
 *
 *>供子类构造函数用。
 * @param {const dialog_highlighter_vtable_t*} vt 虚表对象。
 *
 * @return {dialog_highlighter_t*} 返回对话框高亮策略对象
 */
dialog_highlighter_t *dialog_highlighter_create(const dialog_highlighter_vtable_t *vt);

/**
 * @method dialog_highlighter_set_bg
 * 设置背景图片。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {bitmap_t*} img 背景截图。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_set_bg(dialog_highlighter_t *h, bitmap_t *img);

/**
 * @method dialog_highlighter_set_win
 * 设置底层窗口。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {widget_t*} win 底层窗口。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_set_win(dialog_highlighter_t *h, widget_t *win);

/**
 * @method dialog_highlighter_set_bg_clip_rect
 * 设置背景图片的显示裁剪区。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {rect_t*} clip_rect 背景显示裁剪区。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_set_bg_clip_rect(dialog_highlighter_t *h, rect_t *clip_rect);

/**
 * @method dialog_highlighter_prepare
 * 初始化。在绘制完背景，在截图前调用。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {canvas_t*} c 画布。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_prepare(dialog_highlighter_t *h, canvas_t *c);

/**
 * @method dialog_highlighter_prepare_ex
 * 初始化。在绘制完背景，在截图前调用。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {canvas_t*} c 画布。
 * @param {canvas_t*} canvas_offline 离线画布。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_prepare_ex(dialog_highlighter_t *h, canvas_t *c, canvas_t *canvas_offline);

/**
 * @method dialog_highlighter_set_system_bar_alpha
 * 设置 sytem_bar 的高亮透明值。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {uint8_t} alpha 设置 sytem_bar 的高亮透明值。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_set_system_bar_alpha(dialog_highlighter_t *h, uint8_t alpha);

/**
 * @method dialog_highlighter_get_alpha
 * 获取高亮的透明度。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {float_t} percent 高亮的百分比。
 *
 * @return {uint8_t} 返回透明度。
 */
uint8_t dialog_highlighter_get_alpha(dialog_highlighter_t *h, float_t percent);

/**
 * @method dialog_highlighter_draw
 * 绘制背景。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {float_t} percent 动画进度(0-1)，1表示打开已经完成。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_draw(dialog_highlighter_t *h, float_t percent);

/**
 * @method dialog_highlighter_draw_mask
 * 绘制背景高亮部分。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 * @param {canvas_t*} c 画布。
 * @param {float_t} percent 高亮的百分比。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_draw_mask(dialog_highlighter_t *h, canvas_t *c, float_t percent);

/**
 * @method dialog_highlighter_is_dynamic
 * 是否是动态绘制(方便外层优化)。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 *
 * @return {bool_t} 返回TRUE表示动态绘制，否则表示不是动态绘制。
 */
bool_t dialog_highlighter_is_dynamic(dialog_highlighter_t *h);

/**
 * @method dialog_highlighter_destroy
 * 销毁对话框高亮策略对象。
 * @param {dialog_highlighter_t*} h 对话框高亮策略对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t dialog_highlighter_destroy(dialog_highlighter_t *h);

/*for window manager*/
ret_t dialog_highlighter_clear_image(dialog_highlighter_t *h);

END_C_DECLS

#endif /*TK_DIALOG_HIGHLIGHTER_H*/
