﻿/**
 * File:   image_loader_stb.h
 * Author: AWTK Develop Team
 * Brief:  stb image loader
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
 * 2018-01-21 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_IMAGE_LOADER_STB_H
#define TK_IMAGE_LOADER_STB_H

#include "../base/image_loader.h"

BEGIN_C_DECLS

/**
 * @class image_loader_stb_t
 * @parent image_loader_t
 * stb图片加载器。
 *
 * stb主要用于加载jpg/png/gif等格式的图片，它功能强大，体积小巧。
 *
 * @annotation["fake"]
 *
 */

/**
 * @method image_loader_stb
 * @annotation ["constructor"]
 *
 * 获取stb图片加载器对象。
 *
 * @return {image_loader_t*} 返回图片加载器对象。
 */
image_loader_t *image_loader_stb(void);

/*for tool image_gen only*/

/**
 * @method stb_load_image
 * 加载图片。
 *
 * @annotation ["static"]
 * @param {int32_t} subtype 资源类型。
 * @param {const uint8_t*} buff 资源数据。
 * @param {uint32_t} buff_size 资源数据长度。
 * @param {bitmap_t*} image image 对象。
 * @param {bitmap_format_t} transparent_bitmap_format 带透明通道的位图格式（只能 BITMAP_FMT_RGBA8888 和 BITMAP_FMT_RGBA8888 二选一，其他类型默认都为 BITMAP_FMT_RGBA8888）
 * @param {bitmap_format_t} opaque_bitmap_format 不透明位图格式（暂时支持 BITMAP_FMT_RGBA8888，BITMAP_FMT_RGBA8888，16 位色和 24 位色以及 mono 格式）
 * @param {lcd_orientation_t} o 旋转方向
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t stb_load_image(int32_t subtype, const uint8_t *buff, uint32_t buff_size, bitmap_t *image,
                     bitmap_format_t transparent_bitmap_format,
                     bitmap_format_t opaque_bitmap_format, lcd_orientation_t o);

END_C_DECLS

#endif /*TK_IMAGE_LOADER_STB_H*/
