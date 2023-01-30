﻿/**
 * File:   input_engine.h
 * Author: AWTK Develop Team
 * Brief:  input method engine
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
 * 2018-06-23 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_INPUT_ENGINE_H
#define TK_INPUT_ENGINE_H

#include "../tkc/str.h"
#include "../tkc/buffer.h"
#include "types_def.h"
#include "../input_engines/ime_utils.h"

BEGIN_C_DECLS

typedef ret_t (*input_engine_init_t)(input_engine_t *engine);
typedef ret_t (*input_engine_deinit_t)(input_engine_t *engine);
typedef ret_t (*input_engine_search_t)(input_engine_t *engine, const char *keys);
typedef ret_t (*input_engine_reset_input_t)(input_engine_t *engine);
typedef ret_t (*input_engine_input_t)(input_engine_t *engine, int key);
typedef ret_t (*input_engine_set_lang_t)(input_engine_t *engine, const char *lang);
typedef const char *(*input_engine_get_lang_t)(input_engine_t *engine);

#ifndef TK_IM_MAX_INPUT_CHARS
#define TK_IM_MAX_INPUT_CHARS 15
#endif /*TK_IM_MAX_INPUT_CHARS*/

#define TK_IM_DEFAULT_MAX_CANDIDATE_CHARS 255

#ifndef TK_IM_MAX_CANDIDATE_CHARS
#define TK_IM_MAX_CANDIDATE_CHARS TK_IM_DEFAULT_MAX_CANDIDATE_CHARS
#endif

/**
 * @class input_engine_t
 * 输入法引擎接口。
 *
 * 常见的实现方式有以下几种：
 *
 * * 空实现。用于不需要输入法的嵌入式平台。
 *
 * * 拼音输入法实现。用于需要输入法的嵌入式平台。
 *
 * ```graphviz
 * [default_style]
 *
 * input_engine_pinyin_t -> input_engine_t[arrowhead=empty style=dashed]
 * input_engine_null_t -> input_engine_t[arrowhead=empty style=dashed]
 *
 * ```
 */
struct _input_engine_t
{
  /**
   * @property {str_t} keys
   * @annotation ["private"]
   * 当前的按键组合。
   */
  str_t keys;
  /**
   * @property {wbuffer_t} candidates
   * @annotation ["private"]
   * 当前按键组合对应的候选字列表。
   */
  wbuffer_t candidates;
  /**
   * @property {uint32_t} candidates_nr
   * @annotation ["private"]
   * 当前按键组合对应的候选字/词个数。
   */
  uint32_t candidates_nr;

  /*具体实现需要实现的函数*/
  input_engine_init_t init;
  input_engine_deinit_t deinit;
  input_engine_input_t input;
  input_engine_search_t search;
  input_engine_set_lang_t set_lang;
  input_engine_get_lang_t get_lang;
  input_engine_reset_input_t reset_input;

  /*private*/
  input_method_t *im;
  char lang[TK_NAME_LEN + 1];
  void *data;
};

/**
 * @method input_engine_create
 * @annotation ["constructor"]
 * @param {input_method_t*} im 输入法对象。
 * 创建输入法引擎对象。
 *
 * @return {input_engine_t*} 返回输入法引擎对象。
 */
input_engine_t *input_engine_create(input_method_t *im);

/**
 * @method input_engine_destroy
 * @annotation ["deconstructor"]
 * 销毁输入法引擎对象。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_destroy(input_engine_t *engine);

/**
 * @method input_engine_reset_input
 * 清除所有输入的按键组合和候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_reset_input(input_engine_t *engine);

/**
 * @method input_engine_init
 * 初始化输入法引擎对象。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_init(input_engine_t *engine);

/**
 * @method input_engine_deinit
 * 释放输入法引擎对象。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_deinit(input_engine_t *engine);

/**
 * @method input_engine_reset_candidates
 * 重置输入法引擎对象。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_reset_candidates(input_engine_t *engine);

/**
 * @method input_engine_add_candidate
 * 给输入法引擎对象加入一个候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_add_candidate(input_engine_t *engine, const char *str);

/**
 * @method input_engine_add_candidates_from_char
 * 根据字符类型给输入法引擎对象加入所有符合的候选字。
 * 备注：搜索表是二维数组，需要通过参数 c 来确定使用具体行数。
 *
 * @param {input_engine_t*} engine 输入法引擎对象。
 * @param {const wchar_t**} table 输入法搜索表。
 * @param {char} c 需要显示查询的字符。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_add_candidates_from_char(input_engine_t *engine, const wchar_t **table, char c);

/**
 * @method input_engine_add_candidates_from_string
 * 根据字符串给输入法引擎对象加入所有符合的候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 * @param {const table_entry_t*} items 输入法搜索表。
 * @param {uint32_t} items_nr 输入法搜索表长度。
 * @param {const char*} key 需要显示查询的字符。
 * @param {bool_t} exact 是否只查询完全匹配的字符串。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_add_candidates_from_string(input_engine_t *engine, const table_entry_t *items,
                                              uint32_t items_nr, const char *key, bool_t exact);

/**
 * @method input_engine_dispatch_candidates
 * 请求显示候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 * @param {int32_t} selected 缺省选中候选字的序数。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_dispatch_candidates(input_engine_t *engine, int32_t selected);

/**
 * @engine input_engine_set_lang
 * 设置语言。
 * > 有时在同一种语言环境下，也需要输入多种文字，典型的情况是同时输入中文和英文。
 * > 比如T9输入法，可以同时支持中文和英文输入，配合软键盘随时切换输入的语言。
 * > 数字、小写字母、大写字母和符合也可以视为输入的语言。
 * > 主要用于提示输入法引擎选择适当的输入方法。
 *
 * @annotation ["scriptable"]
 * @param {input_engine_t*} engine 输入法引擎对象。
 * @param {const char*} lang 语言。格式为语言+国家/地区码。如：zh_cn和en_us等。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_set_lang(input_engine_t *engine, const char *lang);

/**
 * @engine input_engine_get_lang
 * 获取语言。
 * @annotation ["scriptable"]
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {const char*} 返回语言。
 */
const char *input_engine_get_lang(input_engine_t *engine);

/**
 * @method input_engine_input
 * 输入新的按键，并更新候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 * @param {int} key 键值。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_input(input_engine_t *engine, int key);

/**
 * @method input_engine_search
 * 根据按键组合，更新候选字，并通过输入法对象提交候选字和pre候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 * @param {const char*} keys 按键组合。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_search(input_engine_t *engine, const char *keys);

END_C_DECLS

#endif /*TK_INPUT_ENGINE_H*/
