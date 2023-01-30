﻿/**
 * File:   input_engine_t9ext.c
 * Author: AWTK Develop Team
 * Brief:  t9 input method engine
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
 * 2020-04-16 Li XianJing <xianjimli@hotmail.com> created
 *
 */

// #include "../tkc/mem.h"
// #include "../tkc/utf8.h"
// #include "../tkc/buffer.h"
// #include "../base/input_engine.h"
// #include "../base/input_method.h"

// #ifdef WITH_IME_T9EXT

// #include "ime_utils.h"
// #include "t9ext_zh_cn.inc"
// #include "pinyin_table.inc"

// typedef enum _input_mode_t
// {
//   INPUT_MODE_DIGIT = 0,
//   INPUT_MODE_LOWER,
//   INPUT_MODE_UPPER,
//   INPUT_MODE_ZH,
//   INPUT_MODE_NR
// } input_mode_t;

// typedef struct _input_engine_t9ext_t
// {
//   input_engine_t input_engine;

//   input_mode_t mode;
//   wbuffer_t pre_candidates;
//   uint32_t pre_candidates_nr;

//   /*for lower/upper*/
//   char last_c;
//   uint32_t index;
//   uint32_t timer_id;
// } input_engine_t9ext_t;

// #ifndef SWITCH_INPUT_MODE_KEY
// #define SWITCH_INPUT_MODE_KEY TK_KEY_ESCAPE
// #endif /*SWITCH_INPUT_MODE_KEY*/

// static ret_t input_engine_t9ext_input(input_engine_t *engine, int key)
// {
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;

//   if (key == SWITCH_INPUT_MODE_KEY)
//   {
//     event_t e = event_init(EVT_IM_LANG_CHANGED, engine->im);

//     t9->mode = (t9->mode + 1) % INPUT_MODE_NR;
//     input_engine_reset_input(engine);

//     t9->index = 0;
//     t9->last_c = 0;
//     if (t9->timer_id != TK_INVALID_ID)
//     {
//       timer_remove(t9->timer_id);
//       t9->timer_id = TK_INVALID_ID;
//       input_method_dispatch_preedit_confirm(engine->im);
//     }

//     input_method_dispatch(engine->im, &e);
//     input_engine_reset_candidates(engine);
//     input_engine_dispatch_candidates(engine, -1);

//     return RET_OK;
//   }

//   if (key >= TK_KEY_0 && key <= TK_KEY_9)
//   {
//     return RET_OK;
//   }

//   if (key == TK_KEY_TAB)
//   {
//     widget_focus_next(engine->im->widget);
//     return RET_OK;
//   }

//   return RET_FAIL;
// }

// static ret_t input_engine_t9ext_reset_input(input_engine_t *engine)
// {
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;

//   input_method_dispatch_pre_candidates(engine->im, (const char *)(t9->pre_candidates.data), 0, -1);

//   return RET_OK;
// }

// static const wchar_t *s_table_num_chars[] = {L"", L"，。、：；？！《》『』（）{}【】",
//                                              L"2ABCabc", L"3DEFdef",
//                                              L"4GHIghi", L"5JKLjkl",
//                                              L"6MNOmno", L"7PQRSpqrs",
//                                              L"8TUVtuv", L"9WXYZwxyz",
//                                              NULL};

// static ret_t input_engine_t9ext_commit_char(input_engine_t *engine, char c)
// {
//   char str[2] = {c, 0};

//   return input_method_commit_text(engine->im, str);
// }

// static ret_t input_engine_t9ext_search_digit(input_engine_t *engine, const char *keys)
// {
//   input_engine_t9ext_commit_char(engine, *keys);
//   input_engine_reset_input(engine);

//   return RET_OK;
// }

// static const wchar_t *s_table_num_lower[] = {L" ", L",.?:/@;:\"\'#$%^&*()_+-={}[]<>|\\",
//                                              L"abc", L"def",
//                                              L"ghi", L"jkl",
//                                              L"mno", L"pqrs",
//                                              L"tuv", L"wxyz",
//                                              NULL};

// static const wchar_t *s_table_num_upper[] = {L" ", L",.?:/@;:\"\'#$%^&*()_+-={}[]<>|\\",
//                                              L"ABC", L"DEF",
//                                              L"GHI", L"JKL",
//                                              L"MNO", L"PQRS",
//                                              L"TUV", L"WXYZ",
//                                              NULL};

// static ret_t input_engine_t9ext_preedit_confirm_timer(const timer_info_t *info)
// {
//   input_engine_t *engine = (input_engine_t *)(info->ctx);
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)(info->ctx);

//   t9->index = 0;
//   t9->last_c = 0;
//   t9->timer_id = TK_INVALID_ID;
//   input_method_dispatch_preedit_confirm(engine->im);
//   input_engine_reset_input(engine);

//   return RET_REMOVE;
// }

// static ret_t input_engine_t9ext_search_alpha(input_engine_t *engine, char c,
//                                              const wchar_t **table)
// {
//   uint32_t i = 0;
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;
//   return_value_if_fail(c >= '0' && c <= '9' && t9 != NULL, RET_FAIL);

//   i = c - '0';
//   if (t9->timer_id == TK_INVALID_ID)
//   {
//     t9->index = 0;
//     t9->last_c = c;
//     input_method_dispatch_preedit(engine->im);
//     t9->timer_id = timer_add(input_engine_t9ext_preedit_confirm_timer, engine, 1000);
//   }
//   else
//   {
//     timer_modify(t9->timer_id, 1000);

//     if (t9->last_c != c)
//     {
//       input_method_dispatch_preedit_confirm(engine->im);

//       t9->index = 0;
//       t9->last_c = c;
//       input_method_dispatch_preedit(engine->im);
//     }
//     else
//     {
//       t9->index = (t9->index + 1) % wcslen(table[i]);
//     }
//   }

//   input_engine_t9ext_commit_char(engine, table[i][t9->index]);
//   engine->keys.size = 0;

//   wbuffer_rewind(&(t9->pre_candidates));
//   t9->pre_candidates_nr = ime_utils_add_chars(&(t9->pre_candidates), table, c, FALSE);
//   input_method_dispatch_pre_candidates(engine->im, (const char *)(t9->pre_candidates.data),
//                                        t9->pre_candidates_nr, t9->index);

//   return RET_OK;
// }

// static ret_t input_engine_t9ext_search_lower(input_engine_t *engine, const char *keys)
// {
//   return input_engine_t9ext_search_alpha(engine, *keys, s_table_num_lower);
// }

// static ret_t input_engine_t9ext_search_upper(input_engine_t *engine, const char *keys)
// {
//   return input_engine_t9ext_search_alpha(engine, *keys, s_table_num_upper);
// }

// static ret_t input_engine_t9ext_search_zh(input_engine_t *engine, const char *keys)
// {
//   const char *first = NULL;
//   uint32_t keys_size = strlen(keys);
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;
//   return_value_if_fail(t9 != NULL, RET_BAD_PARAMS);

//   if (tk_isdigit(keys[0]))
//   {
//     /*map digits to pinyin*/
//     const table_entry_t *items = s_t9ext_numbers_pinyin;
//     uint32_t items_nr = ARRAY_SIZE(s_t9ext_numbers_pinyin);

//     wbuffer_rewind(&(t9->pre_candidates));

//     first = (const char *)(t9->pre_candidates.data);
//     if (keys_size == 1)
//     {
//       t9->pre_candidates_nr = 1;
//       wbuffer_write_string(&(t9->pre_candidates), keys);
//     }
//     else
//     {
//       t9->pre_candidates_nr = 0;
//     }

//     t9->pre_candidates_nr +=
//         ime_utils_table_search(items, items_nr, keys, &(t9->pre_candidates), FALSE, FALSE);
//     if (t9->pre_candidates_nr == 0)
//     {
//       t9->pre_candidates_nr = 1;
//       wbuffer_write_string(&(t9->pre_candidates), keys);
//       input_method_dispatch_pre_candidates(engine->im, (const char *)(t9->pre_candidates.data),
//                                            t9->pre_candidates_nr, 0);
//     }
//     else
//     {
//       input_method_dispatch_pre_candidates(engine->im, (const char *)(t9->pre_candidates.data),
//                                            t9->pre_candidates_nr, 0);

//       input_engine_reset_candidates(engine);
//       if (keys_size == 1)
//       {
//         input_engine_add_candidates_from_char(engine, s_table_num_chars, *keys);
//       }
//       else if (*first)
//       {
//         /*map first pinyin to Chinese chars*/
//         input_engine_add_candidates_from_string(engine, s_pinyin_chinese_items,
//                                                 ARRAY_SIZE(s_pinyin_chinese_items), first, TRUE);
//       }
//       input_engine_dispatch_candidates(engine, -1);
//     }
//   }
//   else
//   {
//     /*map pinyin to Chinese chars*/
//     input_engine_reset_candidates(engine);
//     input_engine_add_candidates_from_string(engine, s_pinyin_chinese_items,
//                                             ARRAY_SIZE(s_pinyin_chinese_items), keys, TRUE);
//     input_engine_dispatch_candidates(engine, -1);
//   }

//   return RET_OK;
// }

// static ret_t input_engine_t9ext_search(input_engine_t *engine, const char *keys)
// {
//   uint32_t keys_size = 0;
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;
//   return_value_if_fail(keys != NULL, RET_FAIL);
//   keys_size = strlen(keys);

//   if (keys_size == 0)
//   {
//     input_engine_reset_input(engine);
//     input_engine_reset_candidates(engine);
//     input_engine_dispatch_candidates(engine, -1);
//     return RET_OK;
//   }

//   log_debug("keys:%s\n", keys);

//   if (t9->mode == INPUT_MODE_DIGIT)
//   {
//     return input_engine_t9ext_search_digit(engine, keys);
//   }

//   if (keys[0] == '0')
//   {
//     input_engine_reset_input(engine);
//     input_method_commit_text(engine->im, " ");

//     return RET_OK;
//   }

//   switch (t9->mode)
//   {
//   case INPUT_MODE_LOWER:
//   {
//     return input_engine_t9ext_search_lower(engine, keys);
//   }
//   case INPUT_MODE_UPPER:
//   {
//     return input_engine_t9ext_search_upper(engine, keys);
//   }
//   default:
//   {
//     if (keys[0] == '1')
//     {
//       t9->pre_candidates_nr = 1;
//       wbuffer_rewind(&(t9->pre_candidates));
//       wbuffer_write_string(&(t9->pre_candidates), keys);
//       input_method_dispatch_pre_candidates(engine->im, (const char *)(t9->pre_candidates.data),
//                                            t9->pre_candidates_nr, 0);

//       input_engine_reset_candidates(engine);
//       input_engine_add_candidates_from_char(engine, s_table_num_chars, '1');
//       input_engine_dispatch_candidates(engine, -1);

//       return RET_OK;
//     }
//     return input_engine_t9ext_search_zh(engine, keys);
//   }
//   }

//   return RET_OK;
// }

// static ret_t input_engine_t9ext_set_lang(input_engine_t *engine, const char *lang)
// {
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;
//   return_value_if_fail(engine != NULL, RET_BAD_PARAMS);

//   event_t e = event_init(EVT_IM_LANG_CHANGED, engine->im);

//   log_debug("input_engine_t9ext_set_lang: %s\n", lang);
//   if (tk_str_eq(lang, "123"))
//   {
//     t9->mode = INPUT_MODE_DIGIT;
//   }
//   else if (tk_str_eq(lang, "abc"))
//   {
//     t9->mode = INPUT_MODE_LOWER;
//   }
//   else if (tk_str_eq(lang, "ABC"))
//   {
//     t9->mode = INPUT_MODE_UPPER;
//   }
//   else if (tk_str_eq(lang, "pinyin"))
//   {
//     t9->mode = INPUT_MODE_ZH;
//   }
//   else
//   {
//     log_debug("not support lang:%s\n", lang);
//   }

//   input_engine_reset_input(engine);

//   t9->index = 0;
//   t9->last_c = 0;
//   if (t9->timer_id != TK_INVALID_ID)
//   {
//     timer_remove(t9->timer_id);
//     t9->timer_id = TK_INVALID_ID;
//     input_method_dispatch_preedit_confirm(engine->im);
//   }

//   input_method_dispatch(engine->im, &e);
//   input_engine_reset_candidates(engine);
//   input_engine_dispatch_candidates(engine, -1);
//   // input_method_dispatch_candidates(engine->im, (const char*)(engine->candidates.data), 0, -1);

//   return RET_OK;
// }

// static const char *input_engine_t9ext_get_lang(input_engine_t *engine)
// {
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;
//   switch (t9->mode)
//   {
//   case INPUT_MODE_DIGIT:
//   {
//     return IM_LANG_DIGIT;
//   }
//   case INPUT_MODE_LOWER:
//   {
//     return IM_LANG_LOWER;
//   }
//   case INPUT_MODE_UPPER:
//   {
//     return IM_LANG_UPPER;
//   }
//   default:
//   {
//     return "pinyin";
//   }
//   }
// }

// static ret_t input_engine_t9ext_init(input_engine_t *engine)
// {
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;
//   return_value_if_fail(engine != NULL && t9 != NULL, RET_BAD_PARAMS);
//   wbuffer_init_extendable(&(t9->pre_candidates));
//   wbuffer_extend_capacity(&(t9->pre_candidates), TK_IM_MAX_CANDIDATE_CHARS);
//   return RET_OK;
// }

// static ret_t input_engine_t9ext_deinit(input_engine_t *engine)
// {
//   input_engine_t9ext_t *t9 = (input_engine_t9ext_t *)engine;
//   return_value_if_fail(engine != NULL && t9 != NULL, RET_BAD_PARAMS);
//   wbuffer_deinit(&(t9->pre_candidates));
//   return RET_OK;
// }

// input_engine_t *input_engine_create(input_method_t *im)
// {
//   input_engine_t9ext_t *t9 = TKMEM_ZALLOC(input_engine_t9ext_t);
//   input_engine_t *engine = (input_engine_t *)t9;

//   return_value_if_fail(engine != NULL, NULL);

//   t9->mode = INPUT_MODE_ZH;
//   str_init(&(engine->keys), TK_IM_MAX_INPUT_CHARS + 1);

//   engine->im = im;
//   engine->init = input_engine_t9ext_init;
//   engine->deinit = input_engine_t9ext_deinit;
//   engine->input = input_engine_t9ext_input;
//   engine->search = input_engine_t9ext_search;
//   engine->set_lang = input_engine_t9ext_set_lang;
//   engine->get_lang = input_engine_t9ext_get_lang;
//   engine->reset_input = input_engine_t9ext_reset_input;

//   return engine;
// }

// ret_t input_engine_destroy(input_engine_t *engine)
// {
//   return_value_if_fail(engine != NULL, RET_BAD_PARAMS);
//   str_reset(&(engine->keys));
//   TKMEM_FREE(engine);

//   return RET_OK;
// }

// #endif /*WITH_IME_T9EXT*/
