﻿/**
 * File:   events.h
 * Author: AWTK Develop Team
 * Brief:  events structs
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

#ifndef TK_EVENTS_H
#define TK_EVENTS_H

#include "../tkc/event.h"
#include "../tkc/value.h"
#include "canvas.h"
#include "system_info.h"

BEGIN_C_DECLS

/**
 * @enum event_type_t
 * @annotation ["scriptable"]
 * @prefix EVT_
 * 类型常量定义。
 */
typedef enum _event_type_t
{
  /**
   * @const EVT_POINTER_DOWN
   * 指针按下事件名(pointer_event_t)。
   */
  EVT_POINTER_DOWN = 0xff,
  /**
   * @const EVT_POINTER_DOWN_BEFORE_CHILDREN
   * 指针按下事件名，在子控件处理之前触发(pointer_event_t)。
   */
  EVT_POINTER_DOWN_BEFORE_CHILDREN,
  /**
   * @const EVT_POINTER_MOVE
   * 指针移动事件名(pointer_event_t)。
   */
  EVT_POINTER_MOVE,
  /**
   * @const EVT_POINTER_MOVE_BEFORE_CHILDREN
   * 指针移动事件名，在子控件处理之前触发(pointer_event_t)。
   */
  EVT_POINTER_MOVE_BEFORE_CHILDREN,
  /**
   * @const EVT_POINTER_UP
   * 指针抬起事件名(pointer_event_t)。
   */
  EVT_POINTER_UP,
  /**
   * @const EVT_POINTER_UP_BEFORE_CHILDREN
   * 指针抬起事件名，在子控件处理之前触发(pointer_event_t)。
   */
  EVT_POINTER_UP_BEFORE_CHILDREN,
  /**
   * @const EVT_WHEEL
   * 滚轮事件名(wheel_event_t)。
   */
  EVT_WHEEL,
  /**
   * @const EVT_WHEEL_BEFORE_CHILDREN
   * 鼠标滚轮事件名，在子控件处理之前触发(wheel_event_t)。
   */
  EVT_WHEEL_BEFORE_CHILDREN,
  /**
   * @const EVT_POINTER_DOWN_ABORT
   * 取消前一个指针按下事件名(pointer_event_t)。
   */
  EVT_POINTER_DOWN_ABORT,
  /**
   * @const EVT_CONTEXT_MENU
   * 右键/长按弹出上下文菜单的事件名(pointer_event_t)。
   */
  EVT_CONTEXT_MENU,
  /**
   * @const EVT_POINTER_ENTER
   * 指针进入事件名(pointer_event_t)。
   */
  EVT_POINTER_ENTER,
  /**
   * @const EVT_POINTER_LEAVE
   * 指针离开事件名(pointer_event_t)。
   */
  EVT_POINTER_LEAVE,
  /**
   * @const EVT_LONG_PRESS
   * 长按事件名(pointer_event_t)。
   */
  EVT_LONG_PRESS,
  /**
   * @const EVT_CLICK
   * 点击事件名(pointer_event_t)。
   */
  EVT_CLICK,
  /**
   * @const EVT_DOUBLE_CLICK
   * 双击事件名(pointer_event_t)。
   */
  EVT_DOUBLE_CLICK,
  /**
   * @const EVT_FOCUS
   * 得到焦点事件名(event_t)。
   */
  EVT_FOCUS,
  /**
   * @const EVT_BLUR
   * 失去焦点事件名(event_t)。
   */
  EVT_BLUR,
  /**
   * @const EVT_KEY_DOWN
   * 键按下事件名(key_event_t)。
   */
  EVT_KEY_DOWN,
  /**
   * @const EVT_KEY_LONG_PRESS
   * 键长按事件名(key_event_t)。
   */
  EVT_KEY_LONG_PRESS,
  /**
   * @const EVT_KEY_DOWN_BEFORE_CHILDREN
   * 键按下事件名，在子控件处理之前触发(key_event_t)。
   */
  EVT_KEY_DOWN_BEFORE_CHILDREN,
  /**
   * @const EVT_KEY_REPEAT
   * 按键repeat事件名(key_event_t)。
   */
  EVT_KEY_REPEAT,
  /**
   * @const EVT_KEY_UP
   * 键抬起事件名(key_event_t)。
   */
  EVT_KEY_UP,
  /**
   * @const EVT_KEY_UP_BEFORE_CHILDREN
   * 键抬起事件名，在子控件处理之前触发(key_event_t)。
   */
  EVT_KEY_UP_BEFORE_CHILDREN,
  /**
   * @const EVT_WILL_MOVE
   * 即将移动Widget的事件名(event_t)。
   */
  EVT_WILL_MOVE,
  /**
   * @const EVT_MOVE
   * 移动Widget的事件名(event_t)。
   */
  EVT_MOVE,
  /**
   * @const EVT_WILL_RESIZE
   * 即将调整Widget大小的事件名(event_t)。
   */
  EVT_WILL_RESIZE,
  /**
   * @const EVT_RESIZE
   * 调整Widget大小的事件名(event_t)。
   */
  EVT_RESIZE,
  /**
   * @const EVT_WILL_MOVE_RESIZE
   * 即将调整Widget大小/位置的事件名(event_t)。
   */
  EVT_WILL_MOVE_RESIZE,
  /**
   * @const EVT_MOVE_RESIZE
   * 调整Widget大小/位置的事件名(event_t)。
   */
  EVT_MOVE_RESIZE,
  /**
   * @const EVT_VALUE_WILL_CHANGE
   * 控件的值即将改变的事件名(value_change_event_t)。
   */
  EVT_VALUE_WILL_CHANGE,
  /**
   * @const EVT_VALUE_CHANGED
   * 控件的值改变的事件名(value_change_event_t)。
   */
  EVT_VALUE_CHANGED,
  /**
   * @const EVT_VALUE_CHANGING
   * 控件的值持续改变(如编辑器正在编辑)的事件名(value_change_event_t)。
   */
  EVT_VALUE_CHANGING,
  /**
   * @const EVT_PAINT
   * 绘制的事件名(paint_event_t)。
   */
  EVT_PAINT,
  /**
   * @const EVT_BEFORE_PAINT
   * 即将绘制的事件名(paint_event_t)。
   */
  EVT_BEFORE_PAINT,
  /**
   * @const EVT_AFTER_PAINT
   * 绘制完成的事件名(paint_event_t)。
   */
  EVT_AFTER_PAINT,
  /**
   * @const EVT_PAINT_DONE
   * 绘制完成(canvas状态已经恢复)的事件名(paint_event_t)。
   */
  EVT_PAINT_DONE,
  /**
   * @const EVT_LOCALE_CHANGED
   * locale改变的事件(event_t)。
   */
  EVT_LOCALE_CHANGED,
  /**
   * @const EVT_ANIM_START
   * 控件动画开始事件(event_t)。
   */
  EVT_ANIM_START,
  /**
   * @const EVT_ANIM_STOP
   * 控件动画被主动停止的事件(event_t)。
   */
  EVT_ANIM_STOP,
  /**
   * @const EVT_ANIM_PAUSE
   * 控件动画被暂停的事件(event_t)。
   */
  EVT_ANIM_PAUSE,
  /**
   * @const EVT_ANIM_ONCE
   * 控件动画yoyo/repeat时，完成一次的事件(event_t)。
   */
  EVT_ANIM_ONCE,
  /**
   * @const EVT_ANIM_END
   * 控件动画完成事件(event_t)。
   */
  EVT_ANIM_END,
  /**
   * @const EVT_WINDOW_LOAD
   * 窗口加载完成事件(event_t)。
   */
  EVT_WINDOW_LOAD,
  /**
   * @const EVT_WIDGET_LOAD
   * 控件加载完成事件(event_t)。
   */
  EVT_WIDGET_LOAD,
  /**
   * @const EVT_WINDOW_WILL_OPEN
   * 窗口即将打开事件(event_t)。
   * 如果有窗口动画，在窗口动画开始前触发。如果没有窗口动画，在窗口被加载后的下一次循环中触发。
   */
  EVT_WINDOW_WILL_OPEN,
  /**
   * @const EVT_WINDOW_OPEN
   * 窗口打开事件(event_t)。
   * 如果有窗口动画，在窗口动画完成时触发。如果没有窗口动画，在窗口被加载后的下一次循环中触发。
   */
  EVT_WINDOW_OPEN,
  /**
   * @const EVT_WINDOW_TO_BACKGROUND
   * 窗口被切换到后台事件(event_t)。
   * 打开新窗口时，当前窗口被切换到后台时，对当前窗口触发本事件。
   */
  EVT_WINDOW_TO_BACKGROUND,
  /**
   * @const EVT_WINDOW_TO_FOREGROUND
   * 窗口被切换到前台事件(event_t)。
   * 关闭当前窗口时，前一个窗口被切换到前台时，对前一个窗口触发本事件。
   */
  EVT_WINDOW_TO_FOREGROUND,
  /**
   * @const EVT_WINDOW_CLOSE
   * 窗口关闭事件。
   */
  EVT_WINDOW_CLOSE,
  /**
   * @const EVT_REQUEST_CLOSE_WINDOW
   * 请求关闭窗口的事件(event_t)。
   */
  EVT_REQUEST_CLOSE_WINDOW,
  /**
   * @const EVT_TOP_WINDOW_CHANGED
   * 顶层窗口改变的事件(window_event_t)。
   */
  EVT_TOP_WINDOW_CHANGED,
  /**
   * @const EVT_IM_START
   * 输入法启动(event_t)。
   */
  EVT_IM_START,
  /**
   * @const EVT_IM_STOP
   * 输入法停止(event_t)。
   */
  EVT_IM_STOP,
  /**
   * @const EVT_IM_COMMIT
   * 输入法提交输入的文本事件(im_commit_event_t)。
   */
  EVT_IM_COMMIT,
  /**
   * @const EVT_IM_CLEAR
   * 清除编辑器内容(event_t)。
   */
  EVT_IM_CLEAR,
  /**
   * @const EVT_IM_CANCEL
   * 取消编辑，恢复之前的内容(event_t)。
   */
  EVT_IM_CANCEL,
  /**
   * @const EVT_IM_PREEDIT
   * 进入预编辑状态(event_t)。
   */
  EVT_IM_PREEDIT,

  /**
   * @const EVT_IM_PREEDIT_CONFIRM
   * 确认预编辑内容，退出预编辑状态(event_t)。
   */
  EVT_IM_PREEDIT_CONFIRM,

  /**
   * @const EVT_IM_PREEDIT_ABORT
   * 删除预编辑内容，退出预编辑状态event_t)。
   */
  EVT_IM_PREEDIT_ABORT,

  /**
   * @const EVT_IM_SHOW_CANDIDATES
   * 输入法请求显示候选字事件(im_candidates_event_t)。
   */
  EVT_IM_SHOW_CANDIDATES,

  /**
   * @const EVT_IM_SHOW_PRE_CANDIDATES
   * 输入法请求显示预候选字事件(im_candidates_event_t)。
   */
  EVT_IM_SHOW_PRE_CANDIDATES,
  /**
   * @const EVT_IM_LANG_CHANGED
   * 输入法语言改变事件(event_t)。
   */
  EVT_IM_LANG_CHANGED,
  /**
   * @const EVT_IM_ACTION
   * 软键盘Action点击事件(event_t)。
   */
  EVT_IM_ACTION,
  /**
   * @const EVT_IM_ACTION_INFO
   * 请求更新软键盘上的Action按钮的信息(im_action_button_info_event_t)。
   */
  EVT_IM_ACTION_INFO,
  /**
   * @const EVT_DRAG_START
   * 开始拖动(event_t)。
   */
  EVT_DRAG_START,
  /**
   * @const EVT_DRAG
   * 拖动(event_t)。
   */
  EVT_DRAG,
  /**
   * @const EVT_DRAG_END
   * 结束拖动(event_t)。
   */
  EVT_DRAG_END,
  /**
   * @const EVT_RESET
   * Reset(event_t)。
   */
  EVT_RESET,
  /**
   * @const EVT_SCREEN_SAVER
   * 在指定的时间内，没有用户输入事件，由窗口管理器触发。
   */
  EVT_SCREEN_SAVER,
  /**
   * @const EVT_LOW_MEMORY
   * 内存不足(event_t)。
   */
  EVT_LOW_MEMORY,
  /**
   * @const EVT_OUT_OF_MEMORY
   * 内存耗尽(event_t)。
   */
  EVT_OUT_OF_MEMORY,
  /**
   * @const EVT_ORIENTATION_WILL_CHANGED
   * 屏幕即将旋转(event_t)。
   */
  EVT_ORIENTATION_WILL_CHANGED,
  /**
   * @const EVT_ORIENTATION_CHANGED
   * 屏幕旋转(event_t)。
   */
  EVT_ORIENTATION_CHANGED,
  /**
   * @const EVT_WIDGET_CREATED
   * 控件创建事件(event_t)。
   */
  EVT_WIDGET_CREATED,
  /**
   * @const EVT_REQUEST_QUIT_APP
   * 请求退出应用程序事件。
   * 点击原生窗口关闭按钮时，通过窗口管理器触发，注册该事件并返回RET_STOP，可以阻止窗口关闭。
   */
  EVT_REQUEST_QUIT_APP,
  /**
   * @const EVT_THEME_WILL_CHANGE
   * 即将改变主题(event_t)。
   */
  EVT_THEME_WILL_CHANGE,
  /**
   * @const EVT_THEME_CHANGED
   * 主题变化(event_t)。
   */
  EVT_THEME_CHANGED,
  /**
   * @const EVT_WIDGET_ADD_CHILD
   * 控件加载新的子控件(event_t)。
   */
  EVT_WIDGET_ADD_CHILD,
  /**
   * @const EVT_WIDGET_REMOVE_CHILD
   * 控件移除子控件(event_t)。
   */
  EVT_WIDGET_REMOVE_CHILD,
  /**
   * @const EVT_SCROLL_START
   * scroll view开始滚动(event_t)。
   */
  EVT_SCROLL_START,
  /**
   * @const EVT_SCROLL
   * scroll view滚动(event_t)。
   */
  EVT_SCROLL,
  /**
   * @const EVT_SCROLL_END
   * scroll view结束滚动(event_t)。
   */
  EVT_SCROLL_END,
  /**
   * @const EVT_MULTI_GESTURE
   * 多点触摸手势(multi_gesture_event_t)。
   */
  EVT_MULTI_GESTURE,
  /**
   * @const EVT_PAGE_CHANGED
   * 页面改变了(event_t)。
   */
  EVT_PAGE_CHANGED,
  /**
   * @const EVT_PAGE_CHANGING
   * 页面正在改变(offset_change_event_t)。
   */
  EVT_PAGE_CHANGING,
  /**
   * @const EVT_ASSET_MANAGER_LOAD_ASSET
   * 资源管理加载某个资源(assets_event_t)。
   */
  EVT_ASSET_MANAGER_LOAD_ASSET,
  /**
   * @const EVT_ASSET_MANAGER_UNLOAD_ASSET
   * 资源管理卸载某个资源(assets_event_t)。
   */
  EVT_ASSET_MANAGER_UNLOAD_ASSET,
  /**
   * @const EVT_ASSET_MANAGER_CLEAR_CACHE
   * 资源管理移除同种资源缓存(assets_event_t)。
   */
  EVT_ASSET_MANAGER_CLEAR_CACHE,
  /**
   * @const EVT_TIMER
   * 定时器(event_t)。
   */
  EVT_TIMER,
  /**
   * @const EVT_DATA
   * 数据到来(event_t)。
   */
  EVT_DATA,
  /**
   * @const EVT_CONNECT
   * 客户连接到来(event_t)。
   */
  EVT_CONNECT,
  /**
   * @const EVT_MODEL_CHANGE
   * 模型变化。用于fscript实现窗口间通讯(model_event_t)。
   */
  EVT_MODEL_CHANGE,
  /**
   * @const EVT_SYSTEM
   * SDL系统事件(system_event_t)。
   */
  EVT_SYSTEM,
  /**
   * @const EVT_REQ_START
   * event queue其它请求编号起始值。
   */
  EVT_REQ_START = 0x1000,
  /**
   * @const EVT_USER_START
   * 用户定义事件起始值。
   */
  EVT_USER_START = 0x2000
} event_type_t;

/**
 * @class model_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * model变化事件。
 */
typedef struct _model_event_t
{
  event_t e;

  /**
   * @property {const char*} name
   * @annotation ["readable", "scriptable"]
   * 模型名称。
   */
  const char *name;

  /**
   * @property {const char*} change_type
   * @annotation ["readable", "scriptable"]
   * 变化类型(update/add/remove)。
   */
  const char *change_type;

  /**
   * @property {tk_object_t*} model
   * @annotation ["readable", "scriptable"]
   * 模型。
   */
  tk_object_t *model;
} model_event_t;

/**
 * @method model_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转model_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {model_event_t*} event对象。
 */
model_event_t *model_event_cast(event_t *event);

/**
 * @method model_event_init
 * 初始化事件。
 * @param {model_event_t*} event event对象。
 * @param {const char*} name 名称。
 * @param {const char*} change_type 变化类型。
 * @param {tk_object_t*} model 模型。
 *
 * @return {event_t*} event对象。
 */
event_t *model_event_init(model_event_t *event, const char *name, const char *change_type,
                          tk_object_t *model);
/**
 * @class wheel_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 滚轮事件。
 */
typedef struct _wheel_event_t
{
  event_t e;
  /**
   * @property {int32_t} dy
   * @annotation ["readable", "scriptable"]
   * 滚轮的y值。
   */
  int32_t dy;
  /**
   * @property {bool_t} alt
   * @annotation ["readable", "scriptable"]
   * alt键是否按下。
   */
  bool_t alt;
  /**
   * @property {bool_t} ctrl
   * @annotation ["readable", "scriptable"]
   * ctrl键是否按下。
   */
  bool_t ctrl;
  /**
   * @property {bool_t} shift
   * @annotation ["readable", "scriptable"]
   * shift键是否按下。
   */
  bool_t shift;
} wheel_event_t;

/**
 * @method wheel_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转wheel_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {wheel_event_t*} event对象。
 */
wheel_event_t *wheel_event_cast(event_t *event);

/**
 * @method wheel_event_init
 * 初始化事件。
 * @param {wheel_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {uint32_t} type 事件类型。
 * @param {int32_t} dy 滚轮的y值。
 *
 * @return {event_t*} event对象。
 */
event_t *wheel_event_init(wheel_event_t *event, uint32_t type, void *target, int32_t dy);

/**
 * @class orientation_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 滚轮事件。
 */
typedef struct _orientation_event_t
{
  event_t e;
  /**
   * @property {int32_t} orientation
   * @annotation ["readable", "scriptable"]
   * 屏幕方向。
   */
  lcd_orientation_t orientation;
  /**
   * @property {int32_t} old_orientation
   * @annotation ["readable", "scriptable"]
   * 旧的屏幕方向。
   */
  lcd_orientation_t old_orientation;
} orientation_event_t;

/**
 * @method orientation_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转orientation_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {orientation_event_t*} event对象。
 */
orientation_event_t *orientation_event_cast(event_t *event);

/**
 * @method orientation_event_init
 * 初始化事件。
 * @param {orientation_event_t*} event event对象。
 * @param {uint32_t} type 事件类型。
 * @param {void*} target 事件目标。
 * @param {lcd_orientation_t} old_orientation 旧的屏幕方向。
 * @param {lcd_orientation_t} new_orientation 新的屏幕方向。
 *
 * @return {event_t*} event对象。
 */
event_t *orientation_event_init(orientation_event_t *event, uint32_t type, void *target,
                                lcd_orientation_t old_orientation,
                                lcd_orientation_t new_orientation);

/**
 * @class value_change_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 值变化事件。
 */
typedef struct _value_change_event_t
{
  event_t e;
  /**
   * @property {value_t} old_value
   * @annotation ["readable"]
   * 旧值。
   */
  value_t old_value;

  /**
   * @property {value_t} new_value
   * @annotation ["readable"]
   * 新值。
   */
  value_t new_value;
} value_change_event_t;

/**
 * @method value_change_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转value_change_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {value_change_event_t*} event对象。
 */
value_change_event_t *value_change_event_cast(event_t *event);

/**
 * @method value_change_event_init
 * 初始化事件。
 * @param {value_change_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {uint32_t} type 事件类型。
 *
 * @return {event_t*} event对象。
 */
event_t *value_change_event_init(value_change_event_t *event, uint32_t type, void *target);

/**
 * @class offset_change_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 值变化事件。
 */
typedef struct _offset_change_event_t
{
  event_t e;
  /**
   * @property {float_t} old_offset
   * @annotation ["readable"]
   * 旧值。
   */
  float_t old_offset;

  /**
   * @property {float_t} new_offset
   * @annotation ["readable"]
   * 新值。
   */
  float_t new_offset;
} offset_change_event_t;

/**
 * @method offset_change_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转offset_change_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {offset_change_event_t*} event对象。
 */
offset_change_event_t *offset_change_event_cast(event_t *event);

/**
 * @method offset_change_event_init
 * 初始化事件。
 * @param {offset_change_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {uint32_t} type 事件类型。
 * @param {float_t} old_offset 旧的偏移数据。
 * @param {float_t} new_offset 新的偏移数据。
 *
 * @return {event_t*} event对象。
 */
event_t *offset_change_event_init(offset_change_event_t *event, uint32_t type, void *target,
                                  float_t old_offset, float_t new_offset);

/**
 * @class pointer_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 指针事件。
 */
typedef struct _pointer_event_t
{
  event_t e;
  /**
   * @property {xy_t} x
   * @annotation ["readable", "scriptable"]
   * x坐标。
   */
  xy_t x;
  /**
   * @property {xy_t} y
   * @annotation ["readable", "scriptable"]
   * y坐标。
   */
  xy_t y;
  /**
   * @property {uint8_t} button
   * @annotation ["readable", "scriptable"]
   * button。
   * 在不同的平台，该属性会发生变化，
   * PC ：左键为 1，中键为 2，右键为 3
   * 嵌入式：默认为 1
   */
  xy_t button;
  /**
   * @property {bool_t} pressed
   * @annotation ["readable", "scriptable"]
   * 指针是否按下。
   */
  bool_t pressed;

  /**
   * @property {bool_t} alt
   * @annotation ["readable", "scriptable"]
   * alt键是否按下。
   */
  bool_t alt;
  /**
   * @property {bool_t} ctrl
   * @annotation ["readable", "scriptable"]
   * ctrl键是否按下。
   */
  bool_t ctrl;
  /**
   * @property {bool_t} cmd
   * @annotation ["readable", "scriptable"]
   * cmd键是否按下。
   */
  bool_t cmd;
  /**
   * @property {bool_t} menu
   * @annotation ["readable", "scriptable"]
   * menu键是否按下。
   */
  bool_t menu;
  /**
   * @property {bool_t} shift
   * @annotation ["readable", "scriptable"]
   * shift键是否按下。
   */
  bool_t shift;
} pointer_event_t;

/**
 * @method pointer_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转pointer_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {pointer_event_t*} event对象。
 */
pointer_event_t *pointer_event_cast(event_t *event);

/**
 * @method pointer_event_init
 * 初始化事件。
 * @param {pointer_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {uint32_t} type 事件类型。
 * @param {int32_t} x x的值。
 * @param {int32_t} y y的值。
 *
 * @return {event_t*} event对象。
 */
event_t *pointer_event_init(pointer_event_t *event, uint32_t type, void *target, int32_t x,
                            int32_t y);

/**
 * @class key_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 按键事件。
 */
typedef struct _key_event_t
{
  event_t e;
  /**
   * @property {uint32_t} key
   * @annotation ["readable", "scriptable"]
   * 键值。
   */
  uint32_t key;
  /**
   * @property {bool_t} alt
   * @annotation ["readable", "scriptable"]
   * alt键是否按下。
   */
  bool_t alt;
  /**
   * @property {bool_t} lalt
   * @annotation ["readable", "scriptable"]
   * left alt键是否按下。
   */
  bool_t lalt;
  /**
   * @property {bool_t} ralt
   * @annotation ["readable", "scriptable"]
   * right alt键是否按下。
   */
  bool_t ralt;
  /**
   * @property {bool_t} ctrl
   * @annotation ["readable", "scriptable"]
   * right alt键是否按下。
   * ctrl键是否按下。
   */
  bool_t ctrl;
  /**
   * @property {bool_t} lctrl
   * @annotation ["readable", "scriptable"]
   * left ctrl键是否按下。
   */
  bool_t lctrl;
  /**
   * @property {bool_t} rctrl
   * @annotation ["readable", "scriptable"]
   * right ctrl键是否按下。
   */
  bool_t rctrl;
  /**
   * @property {bool_t} shift
   * @annotation ["readable", "scriptable"]
   * shift键是否按下。
   */
  bool_t shift;
  /**
   * @property {bool_t} lshift
   * @annotation ["readable", "scriptable"]
   * left shift键是否按下。
   */
  bool_t lshift;
  /**
   * @property {bool_t} rshift
   * @annotation ["readable", "scriptable"]
   * right shift键是否按下。
   */
  bool_t rshift;
  /**
   * @property {bool_t} cmd
   * @annotation ["readable", "scriptable"]
   * left shift键是否按下。
   * cmd/win键是否按下。
   */
  bool_t cmd;
  /**
   * @property {bool_t} menu
   * @annotation ["readable", "scriptable"]
   * menu键是否按下。
   */
  bool_t menu;
  /**
   * @property {bool_t} capslock
   * @annotation ["readable", "scriptable"]
   * capslock键是否按下。
   */
  bool_t capslock;
} key_event_t;

/**
 * @method key_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转key_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {key_event_t*} event对象。
 */
key_event_t *key_event_cast(event_t *event);

/**
 * @method key_event_init
 * 初始化事件。
 * @param {key_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {uint32_t} type 事件类型。
 * @param {int32_t} key key的值。
 *
 * @return {event_t*} event对象。
 */
event_t *key_event_init(key_event_t *event, uint32_t type, void *target, int32_t key);

/**
 * @class paint_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 绘制事件。
 */
typedef struct _paint_event_t
{
  event_t e;
  /**
   * @property {canvas_t*} c
   * @annotation ["readable", "scriptable"]
   * canvas。
   */
  canvas_t *c;
} paint_event_t;

/**
 * @method paint_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转paint_event_t对象。主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {paint_event_t*} event 对象。
 */
paint_event_t *paint_event_cast(event_t *event);

/**
 * @method paint_event_init
 * 初始化事件。
 * @param {paint_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {uint32_t} type 事件类型。
 * @param {canvas_t*} c canvas对象。
 *
 * @return {event_t*} event对象。
 */
event_t *paint_event_init(paint_event_t *event, uint32_t type, void *target, canvas_t *c);

/**
 * @class window_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 窗口事件，由窗口管理器触发。
 */
typedef struct _window_event_t
{
  event_t e;
  /**
   * @property {widget_t*} window
   * @annotation ["readable", "scriptable"]
   * canvas。
   */
  widget_t *window;
} window_event_t;

/**
 * @method window_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转window_event_t对象。主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {window_event_t*} 对象。
 */
window_event_t *window_event_cast(event_t *event);

/**
 * @method window_event_init
 * 初始化事件。
 * @param {window_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {uint32_t} type 事件类型。
 * @param {widget_t*} widget window对象。
 *
 * @return {event_t*} event对象。
 */
event_t *window_event_init(window_event_t *event, uint32_t type, void *target, widget_t *widget);

/**
 * @method pointer_event_rotate
 * 根据屏幕旋转方向修正pointer_event中的坐标。(旋转方向为逆时针)
 * @param {pointer_event_t*} evt 指针事件对象。
 * @param {system_info_t*} info 系统信息。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t pointer_event_rotate(pointer_event_t *evt, system_info_t *info);

/**
 * @class multi_gesture_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 多点触摸手势事件。
 */
typedef struct _multi_gesture_event_t
{
  event_t e;
  /**
   * @property {xy_t} x
   * @annotation ["readable", "scriptable"]
   * 中心点x坐标。
   */
  xy_t x;
  /**
   * @property {xy_t} y
   * @annotation ["readable", "scriptable"]
   * 中心点y坐标。
   */
  xy_t y;
  /**
   * @property {float} rotation
   * @annotation ["readable", "scriptable"]
   * 旋转角度(幅度)增量。（单位弧度）
   */
  float_t rotation;
  /**
   * @property {float} distance
   * @annotation ["readable", "scriptable"]
   * 两点间的距离增量。(-1,0)表示缩小，(0-1)表示增加。
   */
  float_t distance;
} multi_gesture_event_t;

/**
 * @method multi_gesture_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转multi_gesture_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {multi_gesture_event_t*} event对象。
 */
multi_gesture_event_t *multi_gesture_event_cast(event_t *event);

/**
 * @method multi_gesture_event_init
 * 初始化事件。
 * @param {multi_gesture_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {int32_t} x x的值。
 * @param {int32_t} y y的值。
 * @param {float} rotation 旋转角度(幅度)增量。
 * @param {float} distance 两点间的距离增量。(-1,0)表示缩小，(0-1)表示增加。
 *
 * @return {event_t*} event对象。
 */
event_t *multi_gesture_event_init(multi_gesture_event_t *event, void *target, int32_t x, int32_t y,
                                  float rotation, float distance);

/**
 * @class assets_event_t
 * @parent event_t
 * 资源事件，由资源管理器触发。
 */
typedef struct _assets_event_t
{
  event_t e;
  /**
   * @property {asset_type_t} type
   * @annotation ["readable"]
   * 触发事件的资源类型
   */
  asset_type_t type;
  /**
   * @property {asset_info_t*} asset_info
   * @annotation ["readable"]
   * 触发事件的资源对象
   */
  asset_info_t *asset_info;
} assets_event_t;

/**
 * @method assets_event_init
 * 初始化事件。
 * @param {window_event_t*} event event对象。
 * @param {assets_manager_t*} am 事件目标资源管理器。
 * @param {uint32_t} type 事件类型。
 * @param {asset_type_t} asset_type 资源类型。
 * @param {asset_info_t*} asset_info 资源对象。
 *
 * @return {event_t*} event对象。
 */
event_t *assets_event_init(assets_event_t *event, assets_manager_t *am, uint32_t type,
                           asset_type_t asset_type, asset_info_t *asset_info);

/**
 * @class theme_change_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 主题变化事件。
 */
typedef struct _theme_change_event_t
{
  event_t e;
  /**
   * @property {const char*} name
   * @annotation ["readable", "scriptable"]
   * 主题名称。
   */
  const char *name;
} theme_change_event_t;

/**
 * @method theme_change_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转theme_change_event_t对象，主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {theme_change_event_t*}  返回event对象。
 */
theme_change_event_t *theme_change_event_cast(event_t *event);

/**
 * @method theme_change_event_init
 * 初始theme change event。
 *
 * @param {theme_change_event_t*} event event对象。
 * @param {uint32_t} type 事件类型。
 * @param {const char*} name 主题名。
 *
 * @return {event_t*} 返回event对象。
 */
event_t *theme_change_event_init(theme_change_event_t *event, uint32_t type, const char *name);

/**
 * @class system_event_t
 * @annotation ["scriptable"]
 * @parent event_t
 * 系统事件。
 */
typedef struct _system_event_t
{
  event_t e;
  /**
   * @property {void*} sdl_event
   * @annotation ["readable", "scriptable"]
   * SDL_Event。
   */
  void *sdl_event;
} system_event_t;

/**
 * @method system_event_cast
 * @annotation ["cast", "scriptable"]
 * 把event对象转system_event_t对象。主要给脚本语言使用。
 * @param {event_t*} event event对象。
 *
 * @return {system_event_t*} event 对象。
 */
system_event_t *system_event_cast(event_t *event);

/**
 * @method system_event_init
 * 初始化事件。
 * @param {system_event_t*} event event对象。
 * @param {void*} target 事件目标。
 * @param {void*} sdl_event SDL_Event对象。
 *
 * @return {event_t*} event对象。
 */
event_t *system_event_init(system_event_t *event, void *target, void *sdl_event);

/**
 * @class event_t
 * @annotation ["scriptable"]
 * 事件基类。
 */
/**
 * @method event_from_name
 * 将事件名转换成事件的值。
 * @annotation ["scriptable", "static"]
 * @param {const char*} name 事件名。
 *
 * @return {int32_t} 返回事件的值。
 */
int32_t event_from_name(const char *name);

#define STR_ON_EVENT_PREFIX "on:"
#define STR_GLOBAL_EVENT_PREFIX "global"
#define STR_GLOBAL_VARS_CHANGED "global_vars_changed"
#define STR_VALUE_CHANGED_BY_UI "value_changed_by_ui"

END_C_DECLS

#endif /**TK_EVENTS_H*/
