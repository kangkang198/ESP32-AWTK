﻿/**
 * File:   graphic_buffer.c
 * Author: AWTK Develop Team
 * Brief:  graphic_buffer
 *
 * Copyright (c) 2019 - 2022  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2019-10-30 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../tkc/mem.h"
#include "bitmap.h"
#include "graphic_buffer.h"

bool_t graphic_buffer_is_valid_for(graphic_buffer_t *buffer, bitmap_t *bitmap)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && bitmap != NULL, FALSE);

  if (buffer->vt->is_valid_for != NULL)
  {
    return buffer->vt->is_valid_for(buffer, bitmap);
  }
  else
  {
    return TRUE;
  }
}

uint8_t *graphic_buffer_lock_for_read(graphic_buffer_t *buffer)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->lock_for_read != NULL,
                       NULL);

  return buffer->vt->lock_for_read(buffer);
}

uint8_t *graphic_buffer_lock_for_write(graphic_buffer_t *buffer)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->lock_for_write != NULL,
                       NULL);

  return buffer->vt->lock_for_write(buffer);
}

ret_t graphic_buffer_unlock(graphic_buffer_t *buffer)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->unlock != NULL,
                       RET_BAD_PARAMS);

  return buffer->vt->unlock(buffer);
}

ret_t graphic_buffer_attach(graphic_buffer_t *buffer, void *data, uint32_t w, uint32_t h)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->attach != NULL,
                       RET_BAD_PARAMS);

  return buffer->vt->attach(buffer, data, w, h);
}

uint32_t graphic_buffer_get_physical_width(graphic_buffer_t *buffer)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->get_width != NULL, 0);

  return buffer->vt->get_width(buffer);
}

uint32_t graphic_buffer_get_physical_height(graphic_buffer_t *buffer)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->get_height != NULL, 0);

  return buffer->vt->get_height(buffer);
}

uint32_t graphic_buffer_get_physical_line_length(graphic_buffer_t *buffer)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->get_line_length != NULL,
                       0);

  return buffer->vt->get_line_length(buffer);
}

ret_t graphic_buffer_destroy(graphic_buffer_t *buffer)
{
  return_value_if_fail(buffer != NULL && buffer->vt != NULL && buffer->vt->destroy != NULL,
                       RET_BAD_PARAMS);

  return buffer->vt->destroy(buffer);
}
