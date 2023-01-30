﻿/**
 * File:   assets_manager.h
 * Author: AWTK Develop Team
 * Brief:  asset manager
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
 * 2018-03-07 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../tkc/mem.h"
#include "../tkc/path.h"
#include "../tkc/utils.h"
#include "enums.h"
#include "locale_info.h"
#include "system_info.h"
#include "assets_manager.h"
#include "vgcanvas_asset_manager.inc"

#define RAW_DIR "raw"
#define ASSETS_DIR "assets"
#define THEME_DEFAULT "default"

static int asset_cache_cmp_type(const void *a, const void *b)
{
  const asset_info_t *aa = (const asset_info_t *)a;
  const asset_info_t *bb = (const asset_info_t *)b;

  if (aa->is_in_rom)
  {
    return -1;
  }

  return aa->type - bb->type;
}

static int asset_cache_cmp_type_and_name(const void *a, const void *b)
{
  const asset_info_t *aa = (const asset_info_t *)a;
  const asset_info_t *bb = (const asset_info_t *)b;

  if (aa->is_in_rom)
  {
    return -1;
  }

  if (aa->type == bb->type)
  {
    return tk_str_cmp(aa->name, bb->name);
  }
  else
  {
    return aa->type - bb->type;
  }
}

static assets_manager_t *s_assets_manager = NULL;

static ret_t assets_manager_dispatch_event(assets_manager_t *am, int32_t etype,
                                           asset_info_t *info)
{
  assets_event_t e;
  return emitter_dispatch(EMITTER(am),
                          assets_event_init(&e, am, etype, (asset_type_t)(info->type), info));
}

static locale_info_t *assets_manager_get_locale_info(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, NULL);

  return am->locale_info != NULL ? am->locale_info : locale_info();
}

#if defined(AWTK_WEB)
static asset_info_t *assets_manager_load_impl(assets_manager_t *am, asset_type_t type,
                                              uint16_t subtype, const char *name)
{
  asset_info_t *info = TKMEM_ALLOC(sizeof(asset_info_t));
  return_value_if_fail(info != NULL, NULL);

  memset(info, 0x00, sizeof(asset_info_t));
  info->size = 0;
  info->type = type;
  info->subtype = subtype;
  info->refcount = 1;
  info->is_in_rom = FALSE;
  strncpy(info->name, name, TK_NAME_LEN);

  return info;
}
#else
#include "../tkc/fs.h"

static system_info_t *assets_manager_get_system_info(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, NULL);

  return am->system_info != NULL ? am->system_info : system_info();
}

const char *assets_manager_get_res_root(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, NULL);

  if (am->res_root != NULL)
  {
    return am->res_root;
  }
  else
  {
    return assets_manager_get_system_info(am)->app_root;
  }
}

static ret_t build_asset_dir_one_theme(char *path, uint32_t size, const char *res_root,
                                       const char *theme, const char *ratio, const char *subpath)
{
  if (ratio != NULL)
  {
    return_value_if_fail(path_build(path, size, res_root, ASSETS_DIR, theme, RAW_DIR, subpath,
                                    ratio, NULL) == RET_OK,
                         RET_FAIL);
  }
  else
  {
    return_value_if_fail(
        path_build(path, size, res_root, ASSETS_DIR, theme, RAW_DIR, subpath, NULL) == RET_OK,
        RET_FAIL);
  }

  return RET_OK;
}

static ret_t build_asset_filename_one_theme(char *path, uint32_t size, const char *res_root,
                                            const char *theme, const char *ratio,
                                            const char *subpath, const char *name,
                                            const char *extname)
{
  const char *pextname = NULL;
  char sep[2] = {TK_PATH_SEP, 0};

  return_value_if_fail(
      build_asset_dir_one_theme(path, size, res_root, theme, ratio, subpath) == RET_OK, RET_FAIL);
  return_value_if_fail(tk_str_append(path, size, sep) == RET_OK, RET_FAIL);
  return_value_if_fail(tk_str_append(path, size, name) == RET_OK, RET_FAIL);

  pextname = strrchr(name, '.');
  if (pextname == NULL || !tk_str_eq(pextname, extname))
  {
    return_value_if_fail(tk_str_append(path, size, extname) == RET_OK, RET_FAIL);
  }

  return RET_OK;
}

static ret_t build_asset_filename_custom(assets_manager_t *am, char *path, uint32_t size,
                                         const char *theme, const char *ratio, const char *subpath,
                                         const char *name, const char *extname)
{
  if (am->custom_build_asset_dir != NULL)
  {
    char sep[2] = {TK_PATH_SEP, 0};
    return_value_if_fail(am->custom_build_asset_dir(am->custom_build_asset_dir_ctx, path, size,
                                                    theme, ratio, subpath) == RET_OK,
                         RET_FAIL);
    return_value_if_fail(tk_str_append(path, size, sep) == RET_OK, RET_FAIL);
    return_value_if_fail(tk_str_append(path, size, name) == RET_OK, RET_FAIL);
    return_value_if_fail(tk_str_append(path, size, extname) == RET_OK, RET_FAIL);

    return RET_OK;
  }

  return RET_FAIL;
}

static ret_t build_asset_filename_default(char *path, uint32_t size, const char *res_root,
                                          const char *theme, const char *ratio, const char *subpath,
                                          const char *name, const char *extname)
{
  return_value_if_fail(build_asset_filename_one_theme(path, size, res_root, theme, ratio, subpath,
                                                      name, extname) == RET_OK,
                       RET_FAIL);
  return RET_OK;
}

static const char *device_pixel_ratio_to_str(float_t dpr)
{
  const char *ratio = "x1";
  if (dpr >= 3)
  {
    ratio = "x3";
  }
  else if (dpr >= 2)
  {
    ratio = "x2";
  }

  return ratio;
}

ret_t assets_manager_build_asset_filename(assets_manager_t *am, char *path, uint32_t size,
                                          const char *theme, bool_t ratio_sensitive,
                                          const char *subpath, const char *name,
                                          const char *extname)
{
  const char *ratio = NULL;
  const char *res_root = assets_manager_get_res_root(am);
  system_info_t *sysinfo = assets_manager_get_system_info(am);
  return_value_if_fail(sysinfo != NULL, RET_BAD_PARAMS);
  ratio = device_pixel_ratio_to_str(sysinfo->device_pixel_ratio);

  if (ratio_sensitive)
  {
    if (build_asset_filename_custom(am, path, size, theme, ratio, subpath, name, extname) ==
        RET_OK)
    {
      return RET_OK;
    }

    return build_asset_filename_default(path, size, res_root, theme, ratio, subpath, name, extname);
  }
  else
  {
    if (build_asset_filename_custom(am, path, size, theme, NULL, subpath, name, extname) ==
        RET_OK)
    {
      return RET_OK;
    }

    return build_asset_filename_default(path, size, res_root, theme, NULL, subpath, name, extname);
  }
}

static asset_info_t *try_load_image(assets_manager_t *am, const char *theme, const char *name,
                                    asset_image_type_t subtype, bool_t ratio)
{
  char path[MAX_PATH + 1];
  const char *extname = NULL;
  const char *subpath = ratio ? "images" : "images/xx";

  switch (subtype)
  {
  case ASSET_TYPE_IMAGE_JPG:
  {
    extname = ".jpg";
    break;
  }
  case ASSET_TYPE_IMAGE_PNG:
  {
    extname = ".png";
    break;
  }
  case ASSET_TYPE_IMAGE_GIF:
  {
    extname = ".gif";
    break;
  }
  case ASSET_TYPE_IMAGE_BMP:
  {
    extname = ".bmp";
    break;
  }
  case ASSET_TYPE_IMAGE_BSVG:
  {
    extname = ".bsvg";
    subpath = "images/svg";
    break;
  }
  case ASSET_TYPE_IMAGE_OTHER:
  {
    extname = "";
    break;
  }
  default:
  {
    return NULL;
  }
  }

  return_value_if_fail(assets_manager_build_asset_filename(am, path, MAX_PATH, theme, ratio,
                                                           subpath, name, extname) == RET_OK,
                       NULL);

  if (subtype == ASSET_TYPE_IMAGE_JPG && !asset_loader_exist(am->loader, path))
  {
    uint32_t len = strlen(path);
    return_value_if_fail(MAX_PATH > len, NULL);
    memcpy(path + len - 4, ".jpeg", 5);
    path[len + 1] = '\0';
  }

  return asset_loader_load(am->loader, ASSET_TYPE_IMAGE, subtype, path, name);
}

static asset_info_t *try_load_assets(assets_manager_t *am, const char *theme, const char *name,
                                     const char *extname, asset_type_t type, uint16_t subtype)
{
  char path[MAX_PATH + 1];
  const char *subpath = NULL;
  switch (type)
  {
  case ASSET_TYPE_FONT:
  {
    subpath = "fonts";
    break;
  }
  case ASSET_TYPE_SCRIPT:
  {
    subpath = "scripts";
    break;
  }
  case ASSET_TYPE_FLOW:
  {
    subpath = "flows";
    break;
  }
  case ASSET_TYPE_STYLE:
  {
    subpath = "styles";
    break;
  }
  case ASSET_TYPE_STRINGS:
  {
    subpath = "strings";
    break;
  }
  case ASSET_TYPE_UI:
  {
    subpath = "ui";
    break;
  }
  case ASSET_TYPE_XML:
  {
    subpath = "xml";
    break;
  }
  case ASSET_TYPE_DATA:
  {
    subpath = "data";
    break;
  }
  default:
  {
    return NULL;
  }
  }

  return_value_if_fail(assets_manager_build_asset_filename(am, path, MAX_PATH, theme, FALSE,
                                                           subpath, name, extname) == RET_OK,
                       NULL);

  return asset_loader_load(am->loader, type, subtype, path, name);
}

static uint16_t subtype_from_extname(const char *extname)
{
  uint16_t subtype = 0;
  return_value_if_fail(extname != NULL, 0);

  if (tk_str_ieq(extname, ".gif"))
  {
    subtype = ASSET_TYPE_IMAGE_GIF;
  }
  else if (tk_str_ieq(extname, ".png"))
  {
    subtype = ASSET_TYPE_IMAGE_PNG;
  }
  else if (tk_str_ieq(extname, ".bmp"))
  {
    subtype = ASSET_TYPE_IMAGE_BMP;
  }
  else if (tk_str_ieq(extname, ".bsvg"))
  {
    subtype = ASSET_TYPE_IMAGE_BSVG;
  }
  else if (tk_str_ieq(extname, ".jpg"))
  {
    subtype = ASSET_TYPE_IMAGE_JPG;
  }
  else if (tk_str_ieq(extname, ".jpeg"))
  {
    subtype = ASSET_TYPE_IMAGE_JPG;
  }
  else if (tk_str_ieq(extname, ".ttf"))
  {
    subtype = ASSET_TYPE_FONT_TTF;
  }
  else
  {
    log_debug("not supported %s\n", extname);
  }

  return subtype;
}

static asset_info_t *load_asset_from_file(uint16_t type, uint16_t subtype, const char *path,
                                          const char *name)
{
  asset_info_t *info = NULL;
  if (file_exist(path))
  {
    int32_t size = file_get_size(path);
    info = asset_info_create(type, subtype, name, size);
    return_value_if_fail(info != NULL, NULL);

    ENSURE(file_read_part(path, info->data, size, 0) == size);
  }

  return info;
}

asset_info_t *assets_manager_load_file(assets_manager_t *am, asset_type_t type, const char *path)
{
  if (file_exist(path))
  {
    const char *extname = strrchr(path, '.');
    uint16_t subtype = subtype_from_extname(extname);

    return load_asset_from_file(type, subtype, path, path);
  }

  return NULL;
}

bool_t assets_manager_is_save_assets_list(asset_type_t type)
{
  /* 资源列表暂时只保存字体资源，风格资源和字符串资源 */
  if (type == ASSET_TYPE_FONT || type == ASSET_TYPE_STYLE || type == ASSET_TYPE_STRINGS)
  {
    return TRUE;
  }
  return FALSE;
}

static asset_info_t *assets_manager_load_asset(assets_manager_t *am, asset_type_t type,
                                               uint16_t subtype, const char *theme,
                                               const char *name)
{
  asset_info_t *info = NULL;
  switch (type)
  {
  case ASSET_TYPE_FONT:
  {
    if (subtype == 0 || (subtype != 0 && subtype == ASSET_TYPE_FONT_TTF))
    {
      if ((info = try_load_assets(am, theme, name, ".ttf", type, ASSET_TYPE_FONT_TTF)) != NULL)
      {
        break;
      }
    }

    if (subtype == 0 || (subtype != 0 && subtype == ASSET_TYPE_FONT_BMP))
    {
      if ((info = try_load_assets(am, theme, name, ".bin", type, ASSET_TYPE_FONT_BMP)) != NULL)
      {
        break;
      }
    }
    break;
  }
  case ASSET_TYPE_SCRIPT:
  {
    if (subtype == 0 || (subtype != 0 && subtype == ASSET_TYPE_SCRIPT_JS))
    {
      if ((info = try_load_assets(am, theme, name, ".js", type, ASSET_TYPE_SCRIPT_JS)) != NULL)
      {
        break;
      }
    }

    if (subtype == 0 || (subtype != 0 && subtype == ASSET_TYPE_SCRIPT_LUA))
    {
      if ((info = try_load_assets(am, theme, name, ".lua", type, ASSET_TYPE_SCRIPT_LUA)) !=
          NULL)
      {
        break;
      }
    }
    break;
  }
  case ASSET_TYPE_FLOW:
  {
    if ((info = try_load_assets(am, theme, name, ".json", type, ASSET_TYPE_SCRIPT_JS)) != NULL)
    {
      break;
    }
    break;
  }
  case ASSET_TYPE_STYLE:
  {
    if ((info = try_load_assets(am, theme, name, ".bin", type, ASSET_TYPE_STYLE)) != NULL)
    {
      break;
    }
    if ((info = try_load_assets(am, theme, name, ".xml", type, ASSET_TYPE_STYLE)) != NULL)
    {
      break;
    }
    break;
  }
  case ASSET_TYPE_STRINGS:
  {
    if ((info = try_load_assets(am, theme, name, ".bin", type, ASSET_TYPE_STRINGS)) != NULL)
    {
      break;
    }
    break;
  }
  case ASSET_TYPE_IMAGE:
  {
    if (subtype != 0)
    {
      asset_image_type_t sub_type = (asset_image_type_t)subtype;
      if ((info = try_load_image(am, theme, name, sub_type, sub_type != ASSET_TYPE_IMAGE_BSVG)) !=
          NULL)
      {
        break;
      }
    }
    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_PNG, TRUE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_BMP, TRUE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_JPG, TRUE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_GIF, TRUE)) != NULL)
    {
      break;
    }

    /*try ratio-insensitive image.*/
    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_PNG, FALSE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_BMP, FALSE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_JPG, FALSE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_GIF, FALSE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_BSVG, FALSE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_OTHER, TRUE)) != NULL)
    {
      break;
    }

    if ((info = try_load_image(am, theme, name, ASSET_TYPE_IMAGE_OTHER, FALSE)) != NULL)
    {
      break;
    }

    break;
  }
  case ASSET_TYPE_UI:
  {
    if ((info = try_load_assets(am, theme, name, ".bin", type, ASSET_TYPE_UI)) != NULL)
    {
      break;
    }
    if ((info = try_load_assets(am, theme, name, ".xml", type, ASSET_TYPE_UI)) != NULL)
    {
      break;
    }
    break;
  }
  case ASSET_TYPE_XML:
  {
    if ((info = try_load_assets(am, theme, name, ".xml", type, ASSET_TYPE_XML)) != NULL)
    {
      break;
    }
    break;
  }
  case ASSET_TYPE_DATA:
  {
    if ((info = try_load_assets(am, theme, name, "", type, ASSET_TYPE_DATA)) != NULL)
    {
      break;
    }
    break;
  }
  default:
    break;
  }

  if (info != NULL && assets_manager_is_save_assets_list(type))
  {
    assets_manager_add(am, info);
  }

  return info;
}

static asset_info_t *assets_manager_load_impl(assets_manager_t *am, asset_type_t type,
                                              uint16_t subtype, const char *name)
{
  asset_info_t *info = NULL;
  if (am->loader == NULL)
  {
    return NULL;
  }

  if (strncmp(name, STR_SCHEMA_FILE, strlen(STR_SCHEMA_FILE)) == 0)
  {
    info = assets_manager_load_file(am, type, name + strlen(STR_SCHEMA_FILE));
    /* 保持和 assets_manager_load_asset 函数内部中的调用 assets_manager_add 的逻辑一样 */
    if (info != NULL && assets_manager_is_save_assets_list(type))
    {
      assets_manager_add(am, info);
    }
    return info;
  }
  else
  {
    const char *theme = am->theme ? am->theme : THEME_DEFAULT;
    info = assets_manager_load_asset(am, type, subtype, theme, name);
    if (info == NULL && !tk_str_eq(theme, THEME_DEFAULT))
    {
      info = assets_manager_load_asset(am, type, subtype, THEME_DEFAULT, name);
    }
    return info;
  }
}
#endif

assets_manager_t *assets_manager(void)
{
  return s_assets_manager;
}

asset_info_t *assets_manager_load(assets_manager_t *am, asset_type_t type, const char *name)
{
  return assets_manager_load_ex(am, type, 0, name);
}

asset_info_t *assets_manager_load_ex(assets_manager_t *am, asset_type_t type, uint16_t subtype,
                                     const char *name)
{
  asset_info_t *info = NULL;
  return_value_if_fail(am != NULL && name != NULL, NULL);

  if (am->custom_load_asset != NULL)
  {
    return am->custom_load_asset(am->custom_load_asset_ctx, type, subtype, name);
  }
  info = assets_manager_load_impl(am, type, subtype, name);

  if (info == NULL && am->fallback_load_asset != NULL)
  {
    info = am->fallback_load_asset(am->fallback_load_asset_ctx, type, subtype, name);
  }

  if (info != NULL)
  {
    assets_manager_dispatch_event(am, EVT_ASSET_MANAGER_LOAD_ASSET, info);
  }
  return info;
}

ret_t assets_manager_set(assets_manager_t *am)
{
  s_assets_manager = am;

  return RET_OK;
}

assets_manager_t *assets_manager_create(uint32_t init_nr)
{
  assets_manager_t *am = TKMEM_ZALLOC(assets_manager_t);
  emitter_init(EMITTER(am));

  return assets_manager_init(am, init_nr);
}

assets_manager_t *assets_manager_init(assets_manager_t *am, uint32_t init_nr)
{
  return_value_if_fail(am != NULL, NULL);

  darray_init(&(am->assets), init_nr, (tk_destroy_t)asset_info_unref,
              (tk_compare_t)asset_cache_cmp_type);
  assets_manager_set_theme(am, THEME_DEFAULT);

#ifdef WITH_ASSET_LOADER
  am->loader = asset_loader_create();
#endif /*WITH_ASSET_LOADER*/
  am->refcount = 1;
  am->name = NULL;

  return am;
}

ret_t assets_manager_set_res_root(assets_manager_t *am, const char *res_root)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);

  am->res_root = tk_str_copy(am->res_root, res_root);

  return RET_OK;
}

ret_t assets_manager_clear_all(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);

  assets_manager_clear_cache(am, ASSET_TYPE_UI);
  assets_manager_clear_cache(am, ASSET_TYPE_STYLE);
  assets_manager_clear_cache(am, ASSET_TYPE_FONT);

  return darray_clear(&(am->assets));
}

const char *assets_manager_get_theme_name(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, NULL);
  return am->theme;
}

ret_t assets_manager_set_theme(assets_manager_t *am, const char *theme)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);

  am->theme = tk_str_copy(am->theme, theme);
  assets_manager_clear_cache(am, ASSET_TYPE_UI);
  assets_manager_clear_cache(am, ASSET_TYPE_STYLE);
  assets_manager_clear_cache(am, ASSET_TYPE_FONT);

  return RET_OK;
}

ret_t assets_manager_set_system_info(assets_manager_t *am, system_info_t *system_info)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);

  if (system_info != NULL)
  {
    am->system_info = system_info;
  }

  return RET_OK;
}

ret_t assets_manager_set_locale_info(assets_manager_t *am, locale_info_t *locale_info)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);

  if (locale_info != NULL)
  {
    am->locale_info = locale_info;
  }

  return RET_OK;
}

ret_t assets_manager_add(assets_manager_t *am, const void *info)
{
  const asset_info_t *r = (const asset_info_t *)info;
  return_value_if_fail(am != NULL && info != NULL, RET_BAD_PARAMS);
#if LOAD_ASSET_WITH_MMAP
  if (r->is_in_rom)
  {
    // 不支持添加非 mmap 资源的外部资源。
    assert(!" mmap model not supported assets this is in rom ");
  }
#endif
  asset_info_ref((asset_info_t *)r);
  return darray_push(&(am->assets), (void *)r);
}

ret_t assets_manager_add_data(assets_manager_t *am, const char *name, uint16_t type,
                              uint16_t subtype, uint8_t *buff, uint32_t size)
{
  asset_info_t *info = asset_info_create(type, subtype, name, size);
  return_value_if_fail(info != NULL, RET_FAIL);
  memcpy(info->data, buff, size);

  return_value_if_fail(assets_manager_add(am, info) == RET_OK, RET_FAIL);

  return asset_info_unref(info);
}

const asset_info_t *assets_manager_find_in_cache(assets_manager_t *am, asset_type_t type,
                                                 uint16_t subtype, const char *name)
{
  uint32_t i = 0;
  const char *assets_name = NULL;
  const asset_info_t *iter = NULL;
  const asset_info_t **all = NULL;
  return_value_if_fail(am != NULL && name != NULL, NULL);

  assets_name = asset_info_get_formatted_name(name);

  all = (const asset_info_t **)(am->assets.elms);

  for (i = 0; i < am->assets.size; i++)
  {
    iter = all[i];
    if (type == iter->type && strcmp(assets_name, iter->name) == 0 &&
        (subtype == 0 || (subtype != 0 && subtype == iter->subtype)))
    {
      return iter;
    }
  }

  return NULL;
}

static const asset_info_t *assets_manager_ref_impl(assets_manager_t *am, asset_type_t type,
                                                   uint16_t subtype, const char *name)
{
  const asset_info_t *info = assets_manager_find_in_cache(am, type, subtype, name);

  if (info == NULL)
  {
    info = assets_manager_load_ex(am, type, subtype, name);
  }
  else
  {
    asset_info_ref((asset_info_t *)info);
  }

  return info;
}

const asset_info_t *assets_manager_ref(assets_manager_t *am, asset_type_t type, const char *name)
{
  return assets_manager_ref_ex(am, type, 0, name);
}

const asset_info_t *assets_manager_ref_ex(assets_manager_t *am, asset_type_t type, uint16_t subtype,
                                          const char *name)
{
  const asset_info_t *info = NULL;
  locale_info_t *locale_info = assets_manager_get_locale_info(am);

  return_value_if_fail(am != NULL && name != NULL, NULL);
  if (*name == '\0')
  {
    return NULL;
  }

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
      info = assets_manager_ref_impl(am, type, subtype, real_name);
      if (info != NULL)
      {
        return info;
      }
    }

    tk_replace_locale(name, real_name, language);
    info = assets_manager_ref_impl(am, type, subtype, real_name);
    if (info != NULL)
    {
      return info;
    }

    tk_replace_locale(name, real_name, "");
    info = assets_manager_ref_impl(am, type, subtype, real_name);
    if (info != NULL)
    {
      return info;
    }
  }
  else
  {
    info = assets_manager_ref_impl(am, type, subtype, name);
  }

  if (info == NULL && type == ASSET_TYPE_UI)
  {
    const key_type_value_t *kv = asset_type_find_by_value(type);
    const char *asset_type = kv != NULL ? kv->name : "unknown";
    log_warn("!!!Asset [name=%s type=%s] not exist!!!\n", name, asset_type);
    (void)asset_type;
  }

  return info;
}

ret_t assets_manager_unref(assets_manager_t *am, const asset_info_t *info)
{
  if (am == NULL || info == NULL)
  {
    /*asset manager was destroied*/
    return RET_OK;
  }

  if (info->refcount == 1)
  {
    assets_manager_dispatch_event(am, EVT_ASSET_MANAGER_UNLOAD_ASSET, (asset_info_t *)info);
  }

  return asset_info_unref((asset_info_t *)info);
}

ret_t assets_manager_clear_cache_ex(assets_manager_t *am, asset_type_t type, const char *name)
{
  int32_t size = 0;
  asset_info_t info;
  ret_t ret = RET_OK;
  return_value_if_fail(am != NULL && name != NULL, RET_BAD_PARAMS);

  memset(&info, 0x00, sizeof(info));
  if (assets_manager_find_in_cache(am, type, 0, name) == NULL)
  {
    return RET_NOT_FOUND;
  }

  info.type = type;
  tk_strncpy_s(info.name, sizeof(info.name), name, strlen(name));

  size = am->assets.size;
  ret = darray_remove_all(&(am->assets), asset_cache_cmp_type_and_name, &info);

  if (am->assets.size < size)
  {
    assets_manager_dispatch_event(am, EVT_ASSET_MANAGER_UNLOAD_ASSET, &info);
  }

  return ret;
}

ret_t assets_manager_clear_cache(assets_manager_t *am, asset_type_t type)
{
  int32_t size = 0;
  asset_info_t info;
  ret_t ret = RET_OK;

  memset(&info, 0x00, sizeof(info));
  info.type = type;
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);

  size = am->assets.size;
  ret = darray_remove_all(&(am->assets), NULL, &info);

  if (am->assets.size < size)
  {
    assets_manager_dispatch_event(am, EVT_ASSET_MANAGER_CLEAR_CACHE, &info);
  }

  return ret;
}

ret_t assets_manager_preload(assets_manager_t *am, asset_type_t type, const char *name)
{
  asset_info_t *info = assets_manager_load(am, type, name);
  return_value_if_fail(info != NULL, RET_FAIL);
  assets_manager_unref(am, info);

  return RET_OK;
}

ret_t assets_manager_deinit(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);

  asset_loader_destroy(am->loader);
  darray_deinit(&(am->assets));
  TKMEM_FREE(am->name);

  return RET_OK;
}

ret_t assets_manager_set_custom_build_asset_dir(
    assets_manager_t *am, assets_manager_build_asset_dir_t custom_build_asset_dir, void *ctx)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);
  am->custom_build_asset_dir_ctx = ctx;
  am->custom_build_asset_dir = custom_build_asset_dir;

  return RET_OK;
}

ret_t assets_manager_set_custom_load_asset(assets_manager_t *am,
                                           assets_manager_load_asset_t custom_load_asset,
                                           void *ctx)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);
  am->custom_load_asset_ctx = ctx;
  am->custom_load_asset = custom_load_asset;

  return RET_OK;
}

ret_t assets_manager_set_fallback_load_asset(assets_manager_t *am,
                                             assets_manager_load_asset_t fallback_load_asset,
                                             void *ctx)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);
  am->fallback_load_asset_ctx = ctx;
  am->fallback_load_asset = fallback_load_asset;

  return RET_OK;
}

ret_t assets_manager_destroy(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);
  emitter_deinit(EMITTER(am));
  assets_manager_deinit(am);

  if (am == assets_manager())
  {
    assets_manager_set(NULL);
  }
  TKMEM_FREE(am->res_root);
  TKMEM_FREE(am->theme);
  TKMEM_FREE(am);

  return RET_OK;
}

ret_t assets_manager_set_loader(assets_manager_t *am, asset_loader_t *loader)
{
  return_value_if_fail(am != NULL && loader != NULL, RET_BAD_PARAMS);

  if (am->loader != NULL)
  {
    asset_loader_destroy(am->loader);
  }
  am->loader = loader;

  return RET_OK;
}

static const char *s_applet_res_root = NULL;

ret_t assets_managers_set_applet_res_root(const char *res_root)
{
  s_applet_res_root = res_root;

  return RET_OK;
}

bool_t assets_managers_is_applet_assets_supported(void)
{
  return s_applet_res_root != NULL;
}

static darray_t *s_assets_managers = NULL;

static int assets_manager_cmp_by_name(assets_manager_t *am, const char *name)
{
  if (tk_str_eq(am->name, name))
  {
    return 0;
  }

  return -1;
}

static asset_info_t *assets_manager_load_asset_fallback_default(assets_manager_t *am,
                                                                asset_type_t type, uint16_t subtype,
                                                                const char *name)
{
  return assets_manager_load_ex(assets_manager(), type, subtype, name);
}

assets_manager_t *assets_managers_ref(const char *name)
{
  assets_manager_t *am = NULL;
  return_value_if_fail(name != NULL, NULL);

  if (s_assets_managers == NULL)
  {
    s_assets_managers = darray_create(3, (tk_destroy_t)assets_manager_destroy,
                                      (tk_compare_t)assets_manager_cmp_by_name);
  }
  return_value_if_fail(s_assets_managers != NULL, NULL);

  am = (assets_manager_t *)darray_find(s_assets_managers, (void *)name);
  if (am == NULL)
  {
    char res_root[MAX_PATH + 1] = {0};
    am = assets_manager_create(5);
    return_value_if_fail(am != NULL, NULL);
    am->name = tk_strdup(name);
    path_build(res_root, MAX_PATH, s_applet_res_root, name, NULL);

    darray_push(s_assets_managers, am);
    assets_manager_set_res_root(am, res_root);
    assets_manager_set_fallback_load_asset(am, assets_manager_load_asset_fallback_default, NULL);
  }
  else
  {
    am->refcount++;
  }

  return am;
}

ret_t assets_managers_unref(assets_manager_t *am)
{
  return_value_if_fail(am != NULL, RET_BAD_PARAMS);
  return_value_if_fail(s_assets_managers != NULL, RET_BAD_PARAMS);

  assert(am->refcount > 0);
  if (am->refcount == 1)
  {
    darray_remove(s_assets_managers, am);
    if (s_assets_managers->size == 0)
    {
      darray_destroy(s_assets_managers);
      s_assets_managers = NULL;
    }
  }
  else
  {
    am->refcount--;
  }

  return RET_OK;
}

ret_t assets_managers_set_theme(const char *theme)
{
  return_value_if_fail(theme != NULL, RET_BAD_PARAMS);

  assets_manager_set_theme(assets_manager(), theme);

  if (s_assets_managers != NULL)
  {
    uint32_t i = 0;
    for (i = 0; i < s_assets_managers->size; i++)
    {
      assets_manager_t *am = (assets_manager_t *)darray_get(s_assets_managers, i);
      assets_manager_set_theme(am, theme);
    }
  }

  return RET_OK;
}
