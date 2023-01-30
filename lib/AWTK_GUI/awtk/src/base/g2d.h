﻿/**
 * File:   g2d.h
 * Author: AWTK Develop Team
 * Brief:  hardware 2d
 *
 * Copyright (c) 2018 - 2022  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * license file for more details.
 *
 */

/**
 * history:
 * ================================================================
 * 2018-05-07 li xianjing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_G2D_H
#define TK_G2D_H

#include "../tkc/rect.h"
#include "bitmap.h"

BEGIN_C_DECLS

/**
 * @class g2d_t
 *
 * 2D加速接口。
 *
 */
/**
 * @method g2d_fill_rect
 * @export none
 * 用颜色填充指定的区域。
 * @param {bitmap_t*} fb framebuffer对象。
 * @param {const rect_t*} dst 要填充的目标区域。
 * @param {color_t} c 颜色。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败，返回失败则上层用软件实现。
 */
ret_t g2d_fill_rect(bitmap_t* fb, const rect_t* dst, color_t c);

/**
 * @method g2d_copy_image
 * @export none
 * 把图片指定的区域拷贝到framebuffer中。
 * @param {bitmap_t*} fb framebuffer对象。
 * @param {bitmap_t*} img 图片对象。
 * @param {const rect_t*} src 要拷贝的区域。
 * @param {xy_t} dx 目标位置的x坐标。
 * @param {xy_t} dy 目标位置的y坐标。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败，返回失败则上层用软件实现。
 */
ret_t g2d_copy_image(bitmap_t* fb, bitmap_t* img, const rect_t* src, xy_t dx, xy_t dy);

/**
 * @method g2d_rotate_image
 * @export none
 * 把图片指定的区域进行旋转并拷贝到framebuffer相应的区域，本函数主要用于辅助实现横屏和竖屏的切换，一般支持90度旋转即可。
 * @param {bitmap_t*} fb framebuffer对象。
 * @param {bitmap_t*} img 图片对象。
 * @param {const rect_t*} src 要旋转并拷贝的区域。
 * @param {lcd_orientation_t} o 旋转角度(一般支持90度即可)。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败，返回失败则上层用软件实现。
 */
ret_t g2d_rotate_image(bitmap_t* fb, bitmap_t* img, const rect_t* src, lcd_orientation_t o);

/**
 * @method image_rotate_ex
 * @export none
 * 把图片指定的区域进行旋转。
 * @param {bitmap_t*} dst 目标图片对象。
 * @param {bitmap_t*} src 源图片对象。
 * @param {const rect_t*} src_r 要旋转并拷贝的区域。
 * @param {xy_t} dx 目标位置的x坐标。（坐标原点为旋转后的坐标系原点，并非是 dst 的左上角）
 * @param {xy_t} dy 目标位置的y坐标。（坐标原点为旋转后的坐标系原点，并非是 dst 的左上角）
 * @param {lcd_orientation_t} o 旋转角度(一般支持90度即可，旋转方向为逆时针)。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败，返回失败则上层用软件实现。
 */
ret_t g2d_rotate_image_ex(bitmap_t* dst, bitmap_t* src, const rect_t* src_r, xy_t dx, xy_t dy,
                          lcd_orientation_t o);

/**
 * @method g2d_blend_image
 * @export none
 * 把图片指定的区域渲染到framebuffer指定的区域，src的大小和dst的大小不一致则进行缩放。
 * 1.硬件不支持缩放，则返回NOT_IMPL。
 * 2.硬件不支持全局alpha，global_alpha!=0xff时返回NOT_IMPL。
 * @param {bitmap_t*} fb framebuffer对象。
 * @param {bitmap_t*} img 图片对象。
 * @param {const rect_t*} dst 目的区域。
 * @param {const rect_t*} src 源区域。
 * @param {uint8_t} global_alpha 全局alpha。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败，返回失败则上层用软件实现。
 */
ret_t g2d_blend_image(bitmap_t* fb, bitmap_t* img, const rect_t* dst, const rect_t* src,
                      uint8_t global_alpha);

/**
 * @method g2d_blend_image_rotate
 * @export none
 * 把图片指定的区域渲染到framebuffer指定的区域，src的大小和dst的大小不一致则进行缩放以及旋转。
 *
 * @param {bitmap_t*} dst 目标图片对象。
 * @param {bitmap_t*} src 源图片对象。
 * @param {const rectf_t*} dst_r 目的区域。（坐标原点为旋转后的坐标系原点，并非是 dst 的左上角）
 * @param {const rectf_t*} src_r 源区域。
 * @param {uint8_t} global_alpha 全局alpha。
 * @param {lcd_orientation_t} o 旋转角度(一般支持90度即可，旋转方向为逆时针)。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败，返回失败则上层用软件实现。
 */
ret_t g2d_blend_image_rotate(bitmap_t* dst, bitmap_t* src, const rectf_t* dst_r,
                             const rectf_t* src_r, uint8_t alpha, lcd_orientation_t o);

END_C_DECLS

#endif /*TK_G2D_H*/
