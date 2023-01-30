﻿/**
 * File:   list_item_seperator.h
 * Author: AWTK Develop Team
 * Brief:  list_item_seperator
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
 * 2022-05-03 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_LIST_ITEM_SEPERATOR_H
#define TK_LIST_ITEM_SEPERATOR_H

#include "../../widgets/check_button.h"

BEGIN_C_DECLS

/**
 * @class list_item_seperator_t
 * @parent check_button_t
 * @annotation ["scriptable","design","widget"]
 * list_item_seperator。
 * 用来模拟实现风琴控件(accordion)和属性页分组控件。
 *> 当前控件被点击时，显示/隐藏当前控件到下一个分隔符控件之间的控件。
 * list_item_seperator\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于list_item_seperator\_t控件。
 *
 * 在xml中使用"list_item_seperator"标签创建list_item_seperator。如：
 *
 * ```xml
 *     <list_item_seperator radio="true" text="Group2" h="32"/>
 *     <list_item style="empty" children_layout="default(r=1,c=0,ym=1)">
 *       <label w="30%" text="ASCII"/>
 *       <edit w="70%" text="" tips="ascii" input_type="ascii" focused="true" action_text="next"/>
 *     </list_item>
 *     <list_item style="empty" children_layout="default(r=1,c=0,ym=1)">
 *       <label w="30%" text="Int"/>
 *       <edit w="70%" text="" tips="int" input_type="int"/>
 *     </list_item>
 *
 *     <list_item_seperator radio="true" text="Group3" h="32"/>
 *     <list_item style="empty" children_layout="default(r=1,c=0,ym=1)">
 *       <label w="30%" text="Float"/>
 *       <edit w="70%" text="" tips="float" input_type="float"/>
 *     </list_item>
 *     <list_item style="empty" children_layout="default(r=1,c=0,ym=1)">
 *       <label w="30%" text="UFloat"/>
 *       <edit w="70%" text="" tips="unsigned float" input_type="ufloat"/>
 *     </list_item>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如背景颜色等。如：
 *
 * ```xml
 *<list_item_seperator text_color="black" bg_color="#e0e0e0">
 * <style name="default" icon_at="left">
 *   <normal  icon="collapse" />
 *   <pressed icon="collapse" />
 *   <over    icon="collapse" text_color="green"/>
 *   <focused icon="collapse" text_color="green"/>
 *   <normal_of_checked icon="expand" text_color="blue"/>
 *   <pressed_of_checked icon="expand" text_color="blue"/>
 *   <over_of_checked icon="expand" text_color="green"/>
 *   <focused_of_checked icon="expand" text_color="green"/>
 * </style>
 *</list_item_seperator>
 * ```
 */
typedef struct _list_item_seperator_t
{
  check_button_t check_button;
} list_item_seperator_t;

/**
 * @method list_item_seperator_create
 * 创建list_item_seperator对象
 * @annotation ["constructor", "scriptable"]
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} 对象。
 */
widget_t *list_item_seperator_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method list_item_seperator_cast
 * 转换为list_item_seperator对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget list_item_seperator对象。
 *
 * @return {widget_t*} list_item_seperator对象。
 */
widget_t *list_item_seperator_cast(widget_t *widget);

#define LIST_ITEM_SEPERATOR(widget) \
  ((list_item_seperator_t *)(list_item_seperator_cast(WIDGET(widget))))

#define WIDGET_TYPE_LIST_ITEM_SEPERATOR "list_item_seperator"

END_C_DECLS

#endif /*TK_LIST_ITEM_SEPERATOR_H*/
