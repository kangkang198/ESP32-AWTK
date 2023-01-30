﻿/**
 * File:   ui_loader.h
 * Author: AWTK Develop Team
 * Brief:  ui_loader interface
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
 * 2018-02-14 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "../tkc/path.h"
#include "ui_loader.h"
#include "assets_manager.h"
#include "../ui_loader/ui_loader_xml.h"
#include "../ui_loader/ui_loader_default.h"
#include "../ui_loader/ui_builder_default.h"

ret_t ui_loader_load(ui_loader_t *loader, const uint8_t *data, uint32_t size, ui_builder_t *b)
{
  return_value_if_fail(loader != NULL && loader->load != NULL, RET_BAD_PARAMS);
  return_value_if_fail(data != NULL && b != NULL, RET_BAD_PARAMS);

  return loader->load(loader, data, size, b);
}

widget_t *ui_loader_load_widget(const char *name)
{
  return ui_loader_load_widget_with_parent(name, NULL);
}

widget_t *ui_loader_load_widget_with_parent(const char *name, widget_t *parent)
{
  widget_t *root = NULL;
  char rname[128] = {0};
  char applet_name[TK_NAME_LEN + 1] = {0};
  const asset_info_t *ui = NULL;
  ui_builder_t *builder = NULL;
  ui_loader_t *loader = default_ui_loader();
  assets_manager_t *am = assets_manager();
  if (strncmp(name, STR_SCHEMA_FILE, strlen(STR_SCHEMA_FILE)) != 0 &&
      assets_managers_is_applet_assets_supported())
  {
    const char *p = strchr(name, '.');
    if (p != NULL)
    {
      tk_strncpy_s(applet_name, sizeof(applet_name) - 1, name, p - name);
      am = assets_managers_ref(applet_name);
      name = p + 1;
    }
  }
  ui = assets_manager_ref(am, ASSET_TYPE_UI, name);
  return_value_if_fail(ui != NULL, NULL);

  if (strncmp(name, STR_SCHEMA_FILE, strlen(STR_SCHEMA_FILE)) == 0 || ui->data[0] == '<')
  {
    path_basename(name, rname, sizeof(rname) - 1);
    name = rname;
    if (strstr(name, ".xml") != NULL || ui->data[0] == '<')
    {
      loader = xml_ui_loader();
    }
  }

  builder = ui_builder_default_create(name);
  builder->widget = parent;

  ui_loader_load(loader, ui->data, ui->size, builder);
  assets_manager_unref(am, ui);
  root = builder->root;
  ui_builder_destroy(builder);
  if (applet_name[0])
  {
    assets_managers_unref(am);
  }

  return root;
}

widget_t *ui_loader_load_widget_from_xml(widget_t *parent, const char *xml, uint32_t size)
{
  widget_t *root = NULL;
  ui_builder_t *builder = NULL;
  ui_loader_t *loader = xml_ui_loader();
  return_value_if_fail(xml != NULL, NULL);

  builder = ui_builder_default_create("noname");
  builder->widget = parent;

  ui_loader_load(loader, (const uint8_t *)xml, size, builder);
  root = builder->root;
  ui_builder_destroy(builder);

  return root;
}
