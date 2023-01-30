﻿/**
 * File:   system_info.h
 * Author: AWTK Develop Team
 * Brief:  system info
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
 * 2018-04-26 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_SYSTEM_INFO_H
#define TK_SYSTEM_INFO_H

#include "../tkc/object.h"
#include "lcd.h"

BEGIN_C_DECLS

/**
 * @enum system_info_flag_t
 * @annotation ["scriptable"]
 * @prefix SYSTEM_INFO_FLAG_
 * SystemInfo常量定义。
 */
typedef enum _system_info_flag_t
{
  /**
   * @const SYSTEM_INFO_FLAG_NONE
   * 无特殊标志。
   */
  SYSTEM_INFO_FLAG_NONE = 0,
  /**
   * @const SYSTEM_INFO_FLAG_FAST_LCD_PORTRAIT
   * 使用快速旋转功能。
   */
  SYSTEM_INFO_FLAG_FAST_LCD_PORTRAIT = 1,
} system_info_flag_t;

/**
 * @class system_info_t
 * @parent tk_object_t
 * 当前系统的信息。
 */
struct _system_info_t
{
  tk_object_t object;

  /**
   * @property {uint32_t} lcd_w
   * @annotation ["readable"]
   * 显示屏的宽度。
   */
  uint32_t lcd_w;

  /**
   * @property {uint32_t} lcd_h
   * @annotation ["readable"]
   * 显示屏的高度。
   */
  uint32_t lcd_h;

  /**
   * @property {lcd_type_t} lcd_type
   * @annotation ["readable"]
   * 显示屏的类型。
   */
  lcd_type_t lcd_type;

  /**
   * @property {float_t} device_pixel_ratio
   * @annotation ["readable"]
   * 显示屏的密度。
   */
  float_t device_pixel_ratio;

  /**
   * @property {float_t} font_scale
   * @annotation ["readable"]
   * 字体缩放比例，用于实现字体整体放大。
   */
  float_t font_scale;

  /**
   * @property {const char*} default_font
   * @annotation ["readable"]
   * 缺省字体。
   */
  const char *default_font;

  /**
   * @property {lcd_orientation_t} lcd_orientation
   * @annotation ["readable"]
   * 显示屏的旋转角度。
   */
  lcd_orientation_t lcd_orientation;

  /**
   * @property {keyboard_type_t} keyboard_type
   * @annotation ["readable"]
   * 键盘类型。
   */
  keyboard_type_t keyboard_type;

  /**
   * @property {const char*} device_orientation
   * @annotation ["readable", "fake"]
   * 显示屏的方向名称，可选值："portrait"表示竖屏，"landscape"表示横屏。
   *
   * > 只能通过object\_get\_prop函数访问。
   */

  /**
   * @property {const char*} country
   * @annotation ["readable", "fake"]
   * locale中的国家。
   *
   * > 只能通过object\_get\_prop函数访问。
   */

  /**
   * @property {const char*} language
   * @annotation ["readable", "fake"]
   * locale中的语言。
   *
   * > 只能通过object\_get\_prop函数访问。
   */

  /**
   * @property {app_type_t} app_type
   * @annotation ["readable"]
   * 应用程序的类型。
   */
  app_type_t app_type;

  /**
   * @property {const char*} app_name
   * @annotation ["readable"]
   * 应用程序的名称。
   */
  const char *app_name;

  /**
   * @property {const char*} app_root
   * @annotation ["readable"]
   * 应用程序的根目录，用于定位资源文件。
   */
  char *app_root;
  /**
   * @property {uint32_t} flags
   * @annotation ["readable"]
   * 标志。请参考{system_info_flag_t}。
   */
  uint32_t flags;
};

/**
 * @method system_info
 * 获取system_info对象。
 * @alias system_info_instance
 * @annotation ["constructor"]
 * @return {system_info_t*} 返回system_info对象。
 */
system_info_t *system_info(void);

/**
 * @method system_info_init
 * 初始化system_info对象。
 * @annotation ["static"]
 * @param {app_type_t} app_type 应用程序的类型。
 * @param {const char*} app_name 应用程序的名称。
 * @param {const char*} app_root 应用程序的根目录，用于定位资源文件。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_init(app_type_t app_type, const char *app_name, const char *app_root);

/**
 * @method system_info_deinit
 * @annotation ["static"]
 * 释放system_info对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_deinit(void);

/**
 * @method system_info_set_font_scale
 * 设置字体缩放比例。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {float_t} font_scale 字体缩放比例。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_font_scale(system_info_t *info, float_t font_scale);

/**
 * @method system_info_set_app_root
 * 设置app root。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {const char*} app_root app root。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_app_root(system_info_t *info, const char *app_root);

/**
 * @method system_info_set_default_font
 * 设置缺省字体。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {const char*} default_font 缺省字体。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_default_font(system_info_t *info, const char *default_font);

/**
 * @method system_info_set_lcd_w
 * 设置LCD宽度。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {uint32_t} lcd_w 设置LCD宽度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_lcd_w(system_info_t *info, uint32_t lcd_w);

/**
 * @method system_info_set_lcd_h
 * 设置LCD高度。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {uint32_t} lcd_h 设置LCD高度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_lcd_h(system_info_t *info, uint32_t lcd_h);

/**
 * @method system_info_set_lcd_type
 * 设置LCD类型。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {lcd_type_t} lcd_type 设置LCD类型。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_lcd_type(system_info_t *info, lcd_type_t lcd_type);

/**
 * @method system_info_set_lcd_orientation
 * 设置LCD的旋转角度。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {lcd_orientation_t} lcd_orientation 设置LCD的旋转角度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_lcd_orientation(system_info_t *info, lcd_orientation_t lcd_orientation);

/**
 * @method system_info_set_keyboard_type
 * 设置键盘类型。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {keyboard_type_t} keyboard_type 键盘类型。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_keyboard_type(system_info_t *info, keyboard_type_t keyboard_type);

/**
 * @method system_info_set_device_pixel_ratio
 * 设置LCD密度。
 *
 * @param {system_info_t*} info system_info对象。
 * @param {float_t} device_pixel_ratio 设置LCD密度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t system_info_set_device_pixel_ratio(system_info_t *info, float_t device_pixel_ratio);

/**
 * @method system_info_fix_font_name
 * 修正字体名称，如果为NULL，返回缺省字体。
 *
 * @param {const char*} name 字体名称。
 *
 * @return {const char*} 返回有效的字体名称。
 */
const char *system_info_fix_font_name(const char *name);

#define SYSTEM_INFO(obj) ((system_info_t *)(obj))

#define SYSTEM_INFO_PROP_LCD_W "lcd_w"
#define SYSTEM_INFO_PROP_LCD_H "lcd_h"
#define SYSTEM_INFO_PROP_LCD_TYPE "lcd_type"
#define SYSTEM_INFO_PROP_LCD_ORIENTATION "lcd_orientation"
#define SYSTEM_INFO_PROP_DEVICE_PIXEL_RATIO "device_pixel_ratio"
#define SYSTEM_INFO_PROP_DEVICE_ORIENTATION "device_orientation"

#define SYSTEM_INFO_PROP_APP_TYPE "app_type"
#define SYSTEM_INFO_PROP_APP_NAME "app_name"
#define SYSTEM_INFO_PROP_APP_ROOT "app_root"
#define SYSTEM_INFO_PROP_APP_ROOT "app_root"
#define SYSTEM_INFO_PROP_LANGUAGE "language"
#define SYSTEM_INFO_PROP_COUNTRY "country"

/**
 * @method system_info_create
 * 创建system_info对象。
 * @annotation ["static"]
 * @param {app_type_t} app_type 应用程序的类型。
 * @param {const char*} app_name 应用程序的名称。
 * @param {const char*} app_root 应用程序的根目录，用于定位资源文件。
 *
 * @return {system_info_t*} 返回创建的对象指针。
 */
system_info_t *system_info_create(app_type_t app_type, const char *app_name, const char *app_root);
ret_t system_info_set_app_info(system_info_t *info, app_type_t app_type, const char *app_name,
                               const char *app_root);

ret_t system_info_eval_exprs(system_info_t *info, const char *exprs, tk_visit_t on_expr_result,
                             void *ctx);

ret_t system_info_set_app_name(system_info_t *info, const char *app_name);

bool_t tk_is_swap_size_by_orientation(lcd_orientation_t old_orientation,
                                      lcd_orientation_t new_orientation);

END_C_DECLS

#endif /*TK_SYSTEM_INFO_H*/
