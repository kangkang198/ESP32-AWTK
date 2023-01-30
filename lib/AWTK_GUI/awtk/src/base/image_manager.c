﻿/**
 * File:   image_manager.c
 * Author: AWTK Develop Team
 * Brief:  bitmap manager
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
 * 2018-01-14 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../tkc/mem.h"
#include "../tkc/utils.h"
#include "../tkc/time_now.h"
#include "locale_info.h"
#include "image_manager.h"

typedef struct _bitmap_cache_t
{
  bitmap_t image;
  char *name;
  uint32_t access_count;
  uint64_t created_time;
  uint64_t last_access_time;
} bitmap_cache_t;

static int bitmap_cache_cmp_time(bitmap_cache_t *a, bitmap_cache_t *b)
{
  return (a->last_access_time <= b->last_access_time) ? 0 : -1;
}

static int bitmap_cache_cmp_name(bitmap_cache_t *a, bitmap_cache_t *b)
{
  return strcmp(a->name, b->name);
}

static int bitmap_cache_cmp_data(bitmap_cache_t *a, bitmap_cache_t *b)
{
  return (char *)(a->image.buffer) - (char *)(b->image.buffer);
}

static int bitmap_cache_cmp_access_time_dec(bitmap_cache_t *a, bitmap_cache_t *b)
{
  return (b->last_access_time) - (a->last_access_time);
}

static ret_t bitmap_cache_destroy(bitmap_cache_t *cache)
{
  return_value_if_fail(cache != NULL, RET_BAD_PARAMS);
  bitmap_t *image = &(cache->image);
  image_manager_t *imm = image->image_manager;

  if (imm != NULL && image->should_free_data)
  {
    imm->mem_size_of_cached_images -= bitmap_get_mem_size(image);
  }
  log_debug("unload image %s\n", cache->name);
  bitmap_destroy(&(cache->image));
  TKMEM_FREE(cache->name);
  TKMEM_FREE(cache);

  return RET_OK;
}

static image_manager_t *s_image_manager = NULL;
image_manager_t *image_manager()
{
  return s_image_manager;
}

ret_t image_manager_set(image_manager_t *imm)
{
  s_image_manager = imm;

  return RET_OK;
}

image_manager_t *image_manager_create(void)
{
  image_manager_t *imm = TKMEM_ZALLOC(image_manager_t);
  return_value_if_fail(imm != NULL, NULL);

  return image_manager_init(imm);
}

static locale_info_t *image_manager_get_locale_info(image_manager_t *imm)
{
  return_value_if_fail(imm != NULL, NULL);
  locale_info_t *locale = locale_info();

  if (imm->assets_manager != NULL && imm->assets_manager->locale_info != NULL)
  {
    locale = imm->assets_manager->locale_info;
  }

  return locale;
}

image_manager_t *image_manager_init(image_manager_t *imm)
{
  return_value_if_fail(imm != NULL, NULL);

  darray_init(&(imm->images), 0, (tk_destroy_t)bitmap_cache_destroy, NULL);
  imm->assets_manager = assets_manager();
  imm->refcount = 1;
  imm->name = NULL;

  return imm;
}

static ret_t image_manager_clear_cache(image_manager_t *imm)
{
  bitmap_cache_t *iter = NULL;
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);
  if (imm->images.size == 0 || imm->max_mem_size_of_cached_images == 0 ||
      imm->mem_size_of_cached_images < imm->max_mem_size_of_cached_images)
  {
    return RET_OK;
  }

  darray_sort(&(imm->images), (tk_compare_t)bitmap_cache_cmp_access_time_dec);
  do
  {
    iter = (bitmap_cache_t *)darray_pop(&(imm->images));
    bitmap_cache_destroy(iter);
    log_debug("clear cache: mem_size_of_cached_images=%u nr=%u", imm->mem_size_of_cached_images,
              imm->images.size);
  } while (imm->images.size > 0 &&
           imm->mem_size_of_cached_images > imm->max_mem_size_of_cached_images);

  return RET_OK;
}

ret_t image_manager_add(image_manager_t *imm, const char *name, const bitmap_t *image)
{
  bitmap_cache_t *cache = NULL;
  return_value_if_fail(imm != NULL && name != NULL && image != NULL, RET_BAD_PARAMS);

  cache = TKMEM_ZALLOC(bitmap_cache_t);
  return_value_if_fail(cache != NULL, RET_OOM);

  cache->image = *image;
  cache->access_count = 1;
  cache->created_time = time_now_s();
  cache->image.should_free_handle = FALSE;
  cache->name = tk_strdup(name);
  cache->image.name = cache->name;
  cache->last_access_time = cache->created_time;

  cache->image.image_manager = imm;
  if (image->should_free_data)
  {
    imm->mem_size_of_cached_images += bitmap_get_mem_size((bitmap_t *)image);
    image_manager_clear_cache(imm);
  }

  return darray_push(&(imm->images), cache);
}

ret_t image_manager_lookup(image_manager_t *imm, const char *name, bitmap_t *image)
{
  bitmap_cache_t info;
  bitmap_cache_t *iter = NULL;
  return_value_if_fail(imm != NULL && name != NULL && image != NULL, RET_BAD_PARAMS);

  memset(&info, 0x00, sizeof(info));

  info.name = (char *)name;
  imm->images.compare = (tk_compare_t)bitmap_cache_cmp_name;
  iter = darray_find(&(imm->images), &info);

  if (iter != NULL)
  {
    *image = iter->image;
    image->destroy = NULL;
    image->image_manager = imm;
    image->specific_destroy = NULL;
    image->should_free_data = FALSE;

    iter->access_count++;
    iter->last_access_time = time_now_s();

    return RET_OK;
  }

  return RET_NOT_FOUND;
}

ret_t image_manager_update_specific(image_manager_t *imm, bitmap_t *image)
{
  bitmap_cache_t info;
  bitmap_cache_t *iter = NULL;
  return_value_if_fail(image != NULL, RET_BAD_PARAMS);

  if (imm == NULL)
  {
    return RET_FAIL;
  }

  if (image->image_manager != NULL)
  {
    imm = image->image_manager;
  }

  info.image.buffer = image->buffer;
  imm->images.compare = (tk_compare_t)bitmap_cache_cmp_data;
  iter = darray_find(&(imm->images), &info);

  if (iter != NULL)
  {
    iter->image.flags = image->flags;
    iter->image.specific = image->specific;
    iter->image.specific_ctx = image->specific_ctx;
    iter->image.specific_destroy = image->specific_destroy;

    image->specific_destroy = NULL;
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t image_manager_get_bitmap_impl(image_manager_t *imm, const char *name,
                                           bitmap_t *image)
{
  const asset_info_t *res = NULL;
  return_value_if_fail(imm != NULL && name != NULL && image != NULL, RET_BAD_PARAMS);

  memset(image, 0x00, sizeof(bitmap_t));
  if (image_manager_lookup(imm, name, image) == RET_OK)
  {
    return RET_OK;
  }

  res = assets_manager_ref(imm->assets_manager, ASSET_TYPE_IMAGE, name);
  if (res == NULL)
  {
    if (imm->fallback_get_bitmap != NULL)
    {
      return imm->fallback_get_bitmap(imm, name, image);
    }
    return RET_NOT_FOUND;
  }

  memset(image, 0x00, sizeof(bitmap_t));
  if (res->subtype == ASSET_TYPE_IMAGE_RAW)
  {
    const bitmap_header_t *header = (const bitmap_header_t *)res->data;

    image->w = header->w;
    image->h = header->h;
    image->flags = header->flags;
    image->format = header->format;
    image->name = res->name;
    image->image_manager = imm;
    image->orientation =
        (header->flags & BITMAP_FLAG_LCD_ORIENTATION) ? header->orientation : LCD_ORIENTATION_0;
    if (image->orientation != LCD_ORIENTATION_0)
    {
      assert(image->orientation == system_info()->lcd_orientation);
      if (image->orientation == LCD_ORIENTATION_90 || image->orientation == LCD_ORIENTATION_270)
      {
        image->w = header->h;
        image->h = header->w;
      }
    }
    bitmap_set_line_length(image, 0);
    image->buffer = GRAPHIC_BUFFER_CREATE_WITH_DATA(header->data, image->w, image->h,
                                                    (bitmap_format_t)(header->format));
    image->w = header->w;
    image->h = header->h;
    bitmap_set_line_length(image, 0);
    image->should_free_data = image->buffer != NULL;
    image_manager_add(imm, name, image);
    image->should_free_data = FALSE;

    return RET_OK;
  }
  else if (res->subtype != ASSET_TYPE_IMAGE_BSVG)
  {
    ret_t ret = image_loader_load_image(res, image);
    if (ret == RET_OK)
    {
      image_manager_add(imm, name, image);
      assets_manager_unref(imm->assets_manager, res);
    }

    return image_manager_lookup(imm, name, image);
  }
  else
  {
    return RET_NOT_FOUND;
  }
}

typedef struct _imm_expr_info_t
{
  image_manager_t *imm;
  bitmap_t *image;
} imm_expr_info_t;

static ret_t image_manager_on_expr_result(void *ctx, const void *data)
{
  imm_expr_info_t *info = (imm_expr_info_t *)ctx;
  const char *name = (const char *)data;

  return image_manager_get_bitmap_impl(info->imm, name, info->image);
}

ret_t image_manager_get_bitmap_exprs(image_manager_t *imm, const char *exprs, bitmap_t *image)
{
  imm_expr_info_t ctx = {imm, image};

  return system_info_eval_exprs(system_info(), exprs, image_manager_on_expr_result, &ctx);
}

ret_t image_manager_get_bitmap(image_manager_t *imm, const char *name, bitmap_t *image)
{
  return_value_if_fail(imm != NULL && name != NULL && image != NULL, RET_BAD_PARAMS);
  locale_info_t *locale_info = image_manager_get_locale_info(imm);

  if (strstr(name, TK_LOCALE_MAGIC) != NULL)
  {
    char locale[TK_NAME_LEN + 1];
    char real_name[TK_NAME_LEN + 1];
    const char *language = locale_info->language;
    const char *country = locale_info->country;

    if (strlen(language) > 0 && strlen(country) > 0)
    {
      tk_snprintf(locale, sizeof(locale) - 1, "%s_%s", language, country);
      tk_replace_locale(name, real_name, locale);
      if (image_manager_get_bitmap_impl(imm, real_name, image) == RET_OK)
      {
        return RET_OK;
      }
    }

    tk_replace_locale(name, real_name, language);
    if (image_manager_get_bitmap_impl(imm, real_name, image) == RET_OK)
    {
      return RET_OK;
    }

    tk_replace_locale(name, real_name, "");
    if (image_manager_get_bitmap_impl(imm, real_name, image) == RET_OK)
    {
      return RET_OK;
    }

    return RET_FAIL;
  }
  else if (strchr(name, '$') != NULL || strchr(name, ',') != NULL)
  {
    return image_manager_get_bitmap_exprs(imm, name, image);
  }
  else
  {
    return image_manager_get_bitmap_impl(imm, name, image);
  }
}

ret_t image_manager_preload(image_manager_t *imm, const char *name)
{
  bitmap_t image;
  return_value_if_fail(imm != NULL && name != NULL && *name, RET_BAD_PARAMS);

  return image_manager_get_bitmap(imm, name, &image);
}

ret_t image_manager_set_assets_manager(image_manager_t *imm, assets_manager_t *am)
{
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);

  imm->assets_manager = am;

  return RET_OK;
}

bool_t image_manager_has_bitmap(image_manager_t *imm, bitmap_t *image)
{
  bitmap_cache_t b;
  return_value_if_fail(imm != NULL && image != NULL, RET_BAD_PARAMS);

  b.image.buffer = image->buffer;

  if (darray_find_ex(&(imm->images), (tk_compare_t)bitmap_cache_cmp_data, &b) == NULL)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

ret_t image_manager_unload_unused(image_manager_t *imm, uint32_t time_delta_s)
{
  bitmap_cache_t b;
  b.last_access_time = time_now_s() - time_delta_s;
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);

  imm->images.compare = (tk_compare_t)bitmap_cache_cmp_time;
  return darray_remove_all(&(imm->images), NULL, &b);
}

ret_t image_manager_unload_all(image_manager_t *imm)
{
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);

  return darray_clear(&(imm->images));
}

ret_t image_manager_unload_bitmap(image_manager_t *imm, bitmap_t *image)
{
  bitmap_cache_t b;
  return_value_if_fail(imm != NULL && image != NULL, RET_BAD_PARAMS);

  b.image.buffer = image->buffer;
  imm->images.compare = (tk_compare_t)bitmap_cache_cmp_data;

  return darray_remove_all(&(imm->images), NULL, &b);
}

ret_t image_manager_deinit(image_manager_t *imm)
{
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(imm->name);
  darray_deinit(&(imm->images));

  return RET_OK;
}

ret_t image_manager_destroy(image_manager_t *imm)
{
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);

  image_manager_deinit(imm);
  TKMEM_FREE(imm);

  return RET_OK;
}

ret_t image_manager_set_max_mem_size_of_cached_images(image_manager_t *imm, uint32_t max_mem_size)
{
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);

  imm->max_mem_size_of_cached_images = max_mem_size;

  return RET_OK;
}

ret_t image_manager_set_fallback_get_bitmap(image_manager_t *imm,
                                            image_manager_get_bitmap_t fallback_get_bitmap,
                                            void *ctx)
{
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);

  imm->fallback_get_bitmap = fallback_get_bitmap;
  imm->fallback_get_bitmap_ctx = ctx;

  return RET_OK;
}

static darray_t *s_image_managers = NULL;

static int image_manager_cmp_by_name(image_manager_t *imm, const char *name)
{
  if (tk_str_eq(imm->name, name))
  {
    return 0;
  }

  return -1;
}

static ret_t image_manager_fallback_get_bitmap_default(image_manager_t *imm, const char *name,
                                                       bitmap_t *image)
{
  return image_manager_get_bitmap(image_manager(), name, image);
}

image_manager_t *image_managers_ref(const char *name)
{
  image_manager_t *imm = NULL;
  return_value_if_fail(name != NULL, NULL);

  if (s_image_managers == NULL)
  {
    s_image_managers = darray_create(3, (tk_destroy_t)image_manager_destroy,
                                     (tk_compare_t)image_manager_cmp_by_name);
  }
  return_value_if_fail(s_image_managers != NULL, NULL);

  imm = (image_manager_t *)darray_find(s_image_managers, (void *)name);
  if (imm == NULL)
  {
    imm = image_manager_create();
    return_value_if_fail(imm != NULL, NULL);
    imm->name = tk_strdup(name);
    darray_push(s_image_managers, imm);
    image_manager_set_assets_manager(imm, assets_managers_ref(name));
    image_manager_set_fallback_get_bitmap(imm, image_manager_fallback_get_bitmap_default, NULL);
  }
  else
  {
    imm->refcount++;
  }

  return imm;
}

ret_t image_managers_unref(image_manager_t *imm)
{
  return_value_if_fail(imm != NULL, RET_BAD_PARAMS);
  return_value_if_fail(s_image_managers != NULL, RET_BAD_PARAMS);

  assert(imm->refcount > 0);
  if (imm->refcount == 1)
  {
    assets_managers_unref(imm->assets_manager);
    darray_remove(s_image_managers, imm);
    if (s_image_managers->size == 0)
    {
      darray_destroy(s_image_managers);
      s_image_managers = NULL;
    }
  }
  else
  {
    imm->refcount--;
  }

  return RET_OK;
}

ret_t image_managers_unload_all(void)
{
  image_manager_unload_all(image_manager());

  if (s_image_managers != NULL)
  {
    uint32_t i = 0;
    for (i = 0; i < s_image_managers->size; i++)
    {
      image_manager_t *imm = (image_manager_t *)darray_get(s_image_managers, i);
      image_manager_unload_all(imm);
    }
  }

  return RET_OK;
}
