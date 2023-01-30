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

#ifndef TK_ASSETS_MANAGER_H
#define TK_ASSETS_MANAGER_H

#include "../tkc/darray.h"
#include "../tkc/emitter.h"
#include "../tkc/asset_info.h"
#include "types_def.h"
#include "asset_loader.h"

BEGIN_C_DECLS

typedef ret_t (*assets_manager_build_asset_dir_t)(void *ctx, char *path, uint32_t size,
                                                  const char *theme, const char *ratio,
                                                  const char *subpath);

typedef asset_info_t *(*assets_manager_load_asset_t)(assets_manager_t *am, asset_type_t type,
                                                     uint16_t subtype, const char *name);

/**
 * @class assets_manager_t
 * @parent emitter_t
 * @annotation ["scriptable"]
 * 资源管理器。
 * 这里的资源管理器并非Windows下的文件浏览器，而是负责对各种资源，比如字体、窗体样式、图片、界面数据、字符串和其它数据的进行集中管理的组件。引入资源管理器的目的有以下几个：
 *
 * * 让上层不需要了解存储的方式。
 * 在没有文件系统时或者内存紧缺时，把资源转成常量数组直接编译到代码中。在有文件系统而且内存充足时，资源放在文件系统中。在有网络时，资源也可以存放在服务器上(暂未实现)。资源管理器为上层提供统一的接口，让上层而不用关心底层的存储方式。
 *
 * * 让上层不需要了解资源的具体格式。
 * 比如一个名为earth的图片，没有文件系统或内存紧缺，图片直接用位图数据格式存在ROM中，而有文件系统时，则用PNG格式存放在文件系统中。资源管理器让上层不需要关心图片的格式，访问时指定图片的名称即可(不用指定扩展名)。
 *
 * * 让上层不需要了解屏幕的密度。
 * 不同的屏幕密度下需要加载不同的图片，比如MacPro的Retina屏就需要用双倍解析度的图片，否则就出现界面模糊。AWTK以后会支持PC软件和手机软件的开发，所以资源管理器需要为此提供支持，让上层不需关心屏幕的密度。
 *
 * * 对资源进行内存缓存。
 * 不同类型的资源使用方式是不一样的，比如字体和窗体样式加载之后会一直使用，UI文件在生成界面之后就暂时不需要了，PNG文件解码之后就只需要保留解码的位图数据即可。资源管理器配合图片管理器等其它组件实现资源的自动缓存。
 *
 *当从文件系统加载资源时，目录结构要求如下：
 *
 * ```
 * assets/{theme}/raw/
 *  fonts   字体
 *  images  图片
 *    x1   普通密度屏幕的图片。
 *    x2   2倍密度屏幕的图片。
 *    x3   3倍密度屏幕的图片。
 *    xx   密度无关的图片。
 *  strings 需要翻译的字符串。
 *  styles  窗体样式数据。
 *  ui      UI描述数据。
 * ```
 *
 */
struct _assets_manager_t
{
  emitter_t emitter;

  /*private*/
  int refcount;
  char *name;
  char *theme;
  char *res_root;
  darray_t assets;
  locale_info_t *locale_info;
  system_info_t *system_info;

  void *custom_load_asset_ctx;
  assets_manager_load_asset_t custom_load_asset;

  void *fallback_load_asset_ctx;
  assets_manager_load_asset_t fallback_load_asset;

  void *custom_build_asset_dir_ctx;
  assets_manager_build_asset_dir_t custom_build_asset_dir;

  asset_loader_t *loader;
};

/**
 * @method assets_manager
 * 获取缺省资源管理器。
 * @alias assets_manager_instance
 * @annotation ["constructor", "scriptable"]
 *
 * @return {assets_manager_t*} 返回asset manager对象。
 */
assets_manager_t *assets_manager(void);

/**
 * @method assets_manager_set
 * 设置缺省资源管理器。
 * @param {assets_manager_t*} am asset manager对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set(assets_manager_t *am);

/**
 * @method assets_manager_create
 * 创建资源管理器。
 * @annotation ["constructor"]
 * @param {uint32_t} init_nr 预先分配资源的个数。
 *
 * @return {assets_manager_t*} 返回asset manager对象。
 */
assets_manager_t *assets_manager_create(uint32_t init_nr);

/**
 * @method assets_manager_init
 * 初始化资源管理器。
 * @annotation ["constructor"]
 * @param {assets_manager_t*} am asset manager对象。
 * @param {uint32_t} init_nr 预先分配资源的个数。
 *
 * @return {assets_manager_t*} 返回asset manager对象。
 */
assets_manager_t *assets_manager_init(assets_manager_t *am, uint32_t init_nr);

/**
 * @method assets_manager_get_res_root
 * 获取资源所在的目录(其下目录结构请参考demos)。
 * @param {assets_manager_t*} am asset manager对象。
 *
 * @return {const char*} 返回资源所在的目录。
 */
const char *assets_manager_get_res_root(assets_manager_t *am);

/**
 * @method assets_manager_set_res_root
 * 设置资源所在的目录(其下目录结构请参考demos)。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {const char*} res_root 资源所在的目录。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_res_root(assets_manager_t *am, const char *res_root);

/**
 * @method assets_manager_set_theme
 * 设置当前的主题。
 * @annotation ["scriptable"]
 * @param {assets_manager_t*} am asset manager对象。
 * @param {const char*} theme 主题名称。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_theme(assets_manager_t *am, const char *theme);

/**
 * @method assets_manager_set_system_info
 * 设置system_info对象。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {system_info_t*} system_info system_info对象。。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_system_info(assets_manager_t *am, system_info_t *system_info);

/**
 * @method assets_manager_set_locale_info
 * 设置locale_info对象。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {locale_info_t*} locale_info locale_info对象。。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_locale_info(assets_manager_t *am, locale_info_t *locale_info);

/**
 * @method assets_manager_add
 * 向资源管理器中增加一个资源。
 * 备注：同一份资源多次调用会出现缓存叠加的问题，导致内存泄露
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_info_t} info 待增加的资源。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_add(assets_manager_t *am, const void *info);

/**
 * @method assets_manager_add_data
 * 向资源管理器中增加一个资源data。
 * 备注：同一份资源多次调用会出现缓存叠加的问题，导致内存泄露
 * @param {assets_manager_t*} am asset manager对象。
 * @param {const char*} name 待增加的资源的名字。
 * @param {uint16_t} type 待增加的资源的主类型枚举。
 * @param {uint16_t} subtype 待增加的资源的子类型枚举。
 * @param {uint8_t*} buff 待增加的资源的data数据。
 * @param {uint32_t} size 待增加的资源的data数据长度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_add_data(assets_manager_t *am, const char *name, uint16_t type,
                              uint16_t subtype, uint8_t *buff, uint32_t size);

/**
 * @method assets_manager_ref
 * 在资源管理器的缓存中查找指定的资源并引用它，如果缓存中不存在，尝试加载该资源。
 * @annotation ["scriptable"]
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 * @param {char*} name 资源的名称。
 *
 * @return {asset_info_t*} 返回资源。
 */
const asset_info_t *assets_manager_ref(assets_manager_t *am, asset_type_t type, const char *name);

/**
 * @method assets_manager_ref_ex
 * 在资源管理器的缓存中查找指定的资源并引用它，如果缓存中不存在，尝试加载该资源。
 * @annotation ["scriptable"]
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 * @param {uint16_t} subtype 资源的子类型。
 * @param {char*} name 资源的名称。
 *
 * @return {asset_info_t*} 返回资源。
 */
const asset_info_t *assets_manager_ref_ex(assets_manager_t *am, asset_type_t type, uint16_t subtype,
                                          const char *name);

/**
 * @method assets_manager_unref
 * 释放指定的资源。
 * @annotation ["scriptable"]
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_info_t*} info 资源。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_unref(assets_manager_t *am, const asset_info_t *info);

/**
 * @method assets_manager_find_in_cache
 * 在资源管理器的缓存中查找指定的资源(不引用)。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 * @param {uint16_t} subtype 资源的子类型。
 * @param {char*} name 资源的名称。
 *
 * @return {asset_info_t*} 返回资源。
 */
const asset_info_t *assets_manager_find_in_cache(assets_manager_t *am, asset_type_t type,
                                                 uint16_t subtype, const char *name);
/**
 * @method assets_manager_load
 * 从文件系统中加载指定的资源，并缓存到内存中。在定义了宏WITH\_FS\_RES时才生效。
 * 备注：内部使用的，如果是加载资源的话，建议使用 assets_manager_ref 函数。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 * @param {const char*} name 资源的名称。
 *
 * @return {asset_info_t*} 返回资源。
 */
asset_info_t *assets_manager_load(assets_manager_t *am, asset_type_t type, const char *name);

/**
 * @method assets_manager_load_ex
 * 从文件系统中加载指定的资源，并缓存到内存中。在定义了宏WITH\_FS\_RES时才生效。
 * 备注：内部使用的，如果是加载资源的话，建议使用 assets_manager_ref_ex 函数。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 * @param {uint16_t} subtype 资源的子类型。
 * @param {char*} name 资源的名称。
 *
 * @return {asset_info_t*} 返回资源。
 */
asset_info_t *assets_manager_load_ex(assets_manager_t *am, asset_type_t type, uint16_t subtype,
                                     const char *name);

/**
 * @method assets_manager_preload
 * 从文件系统中加载指定的资源，并缓存到内存中。在定义了宏WITH\_FS\_RES时才生效。
 * 备注：内部使用的，不建议用户自行调用。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 * @param {char*} name 资源的名称。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_preload(assets_manager_t *am, asset_type_t type, const char *name);

/**
 * @method assets_manager_set_loader
 * 设置loader。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_loader_t*} loader 加载器(由assets manager销毁)。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_loader(assets_manager_t *am, asset_loader_t *loader);

/**
 * @method assets_manager_set_custom_build_asset_dir
 * 设置一个函数，该函数用于生成资源路径。
 *
 * > 有时我们需要优先加载用户自定义的资源，加载失败才加载系统缺省的，可用设置一个函数去实现这类功能。
 *
 * @param {assets_manager_t*} am asset manager对象。
 * @param {assets_manager_build_asset_dir_t} custom_build_asset_dir 回调函数。
 * @param {void*} ctx 回调函数的上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_custom_build_asset_dir(
    assets_manager_t *am, assets_manager_build_asset_dir_t custom_build_asset_dir, void *ctx);

/**
 * @method assets_manager_set_custom_load_asset
 * 设置一个函数，该函数用于实现自定义加载资源。
 *
 * > 如果不支持文件系统，开发者可以设置一个加载资源的回调函数，从flash或其它地方读取资源。
 *
 * @param {assets_manager_t*} am asset manager对象。
 * @param {assets_manager_load_asset_t} custom_load_asset 回调函数。
 * @param {void*} ctx 回调函数的上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_custom_load_asset(assets_manager_t *am,
                                           assets_manager_load_asset_t custom_load_asset,
                                           void *ctx);

/**
 * @method assets_manager_set_fallback_load_asset
 * 设置一个函数，该函数在找不到资源时加载后补资源。
 *
 * @param {assets_manager_t*} am asset manager对象。
 * @param {assets_manager_load_asset_t} fallback_load_asset 回调函数。
 * @param {void*} ctx 回调函数的上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_set_fallback_load_asset(assets_manager_t *am,
                                             assets_manager_load_asset_t fallback_load_asset,
                                             void *ctx);
/**
 * @method assets_manager_clear_cache
 * 清除指定类型的缓存。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_clear_cache(assets_manager_t *am, asset_type_t type);

/**
 * @method assets_manager_clear_cache_ex
 * 清除指定类型和名称的缓存。
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源的类型。
 * @param {const char*} name 资源的名称。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_clear_cache_ex(assets_manager_t *am, asset_type_t type, const char *name);

/**
 * @method assets_manager_clear_all
 * 清除全部缓存的资源。
 * @param {assets_manager_t*} am asset manager对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_clear_all(assets_manager_t *am);

/**
 * @method assets_manager_deinit
 * @param {assets_manager_t*} am asset manager对象。
 * 释放全部资源。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_deinit(assets_manager_t *am);

/**
 * @method assets_manager_destroy
 * @param {assets_manager_t*} am asset manager对象。
 * 释放全部资源并销毁asset manager对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_manager_destroy(assets_manager_t *am);

/**
 * @method assets_manager_load_file
 * @param {assets_manager_t*} am asset manager对象。
 * @param {asset_type_t} type 资源类型。
 * @param {const char*} path 目录。
 * 获取path里的资源。
 *
 * @return {asset_info_t*} 返回asset_info_t。
 */
asset_info_t *assets_manager_load_file(assets_manager_t *am, asset_type_t type, const char *path);

/*public for test*/

/**
 * @method assets_manager_is_save_assets_list
 * 检查指定类型是否需要保存。
 * @param {asset_type_t} type 资源类型。
 *
 * @return {bool_t} 返回TRUE表示需要保持，否则不需要保存。
 */
bool_t assets_manager_is_save_assets_list(asset_type_t type);

/**
 * @class assets_managers_t
 * @annotation ["fake"]
 * 在某些情况下，需要多个资源管理器。比如在手表系统里，每个小应用或表盘，可能放在独立的资源包中，
 * 此时优先加载应用自己的资源，如果没有就加载系统的资源。
 * > 通常AWTK是单进程应用程序，为了避免概念混淆，我们把这些独立可安装的小应用成为"applet"。
 */

/**
 * @method assets_managers_set_applet_res_root
 * 设置小应用程序(applet)的资源根目录。
 * @param {const char*} res_root 资源根目录。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_managers_set_applet_res_root(const char *res_root);

/**
 * @method assets_managers_is_applet_assets_supported
 * 是否支持小应用程序(applet)拥有独立资源目录。
 *
 * @return {bool_t} 返回TRUE表示支持，否则表示不支持。
 */
bool_t assets_managers_is_applet_assets_supported(void);

/**
 * @method assets_managers_ref
 * 获取指定小应用程序(applet)的资源管理器。
 * @annotation ["constructor"]
 * @param {const char*} name 小应用程序(applet)的名称。
 *
 * @return {assets_manager_t*} 返回asset manager对象。
 */
assets_manager_t *assets_managers_ref(const char *name);

/**
 * @method assets_managers_set_theme
 * 设置当前的主题。
 * @annotation ["scriptable"]
 * @param {const char*} theme 主题名称。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_managers_set_theme(const char *theme);

/**
 * @method assets_managers_unref
 * 释放指定小应用程序(applet)的资源管理器。
 * @annotation ["deconstructor"]
 * @param {assets_manager_t*} am 资源管理器对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t assets_managers_unref(assets_manager_t *am);

/**
 * @method assets_manager_get_theme_name
 * 获取当前的主题名称。
 * @annotation ["deconstructor"]
 * @param {assets_manager_t*} am 资源管理器对象。
 *
 * @return {const char*} 返回主题名称。
 */
const char *assets_manager_get_theme_name(assets_manager_t *am);

END_C_DECLS

#endif /*TK_ASSETS_MANAGER_H*/
