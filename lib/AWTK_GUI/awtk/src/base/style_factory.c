﻿/**
 * File:   style_factory.h
 * Author: AWTK Develop Team
 * Brief:  style factory
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
 * 2018-10-27 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "theme.h"
#include "style_const.h"
#include "style_factory.h"

style_t *style_factory_create_style(style_factory_t *factory, const char *style_type)
{
  if (factory != NULL && factory->create_style != NULL)
  {
    return factory->create_style(factory, style_type);
  }
  else
  {
    return style_const_create();
  }
}

static style_factory_t *s_style_factory;
style_factory_t *style_factory(void)
{
  return s_style_factory;
}

ret_t style_factory_set(style_factory_t *factory)
{
  s_style_factory = factory;

  return RET_OK;
}
