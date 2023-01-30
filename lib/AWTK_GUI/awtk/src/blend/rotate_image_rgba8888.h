﻿/**
 * File:   rotate_image_rgba8888.c
 * Author: AWTK Develop Team
 * Brief:  rotate on rgba8888
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
 * 2022-02-24 Generated by gen.sh(DONT MODIFY IT)
 *
 */
#ifndef TK_ROTATE_IMAGE_RGBA8888_H
#define TK_ROTATE_IMAGE_RGBA8888_H

#include "../base/bitmap.h"

ret_t rotate_rgba8888_image(bitmap_t *fb, bitmap_t *img, const rect_t *src, lcd_orientation_t o);

ret_t rotate_rgba8888_image_ex(bitmap_t *fb, bitmap_t *img, const rect_t *src, xy_t dx, xy_t dy,
                               lcd_orientation_t o);

#endif /*TK_ROTATE_IMAGE_RGBA8888_H*/
