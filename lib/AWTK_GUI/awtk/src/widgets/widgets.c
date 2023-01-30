/**
 * File:   widgets.c
 * Author: AWTK Develop Team
 * Brief:  register widgets
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
 * 2018-06-16 Li XianJing <xianjimli@hotmail.com> created
 *
 */
#include "../base/widget_factory.h"

#include "../base/window.h"
#include "../base/dialog.h"
#ifndef AWTK_NOGUI
#include "image.h"
#include "label.h"
#include "button.h"
#include "slider.h"
#include "pages.h"
#include "popup.h"
#include "button_group.h"
#include "group_box.h"
#include "dialog_title.h"
#include "dialog_client.h"
#include "check_button.h"
#include "progress_bar.h"
#include "color_tile.h"
#include "clip_view.h"

#ifndef AWTK_LITE
#include "system_bar.h"
#include "calibration_win.h"
#include "dragger.h"
#include "tab_button.h"
#include "tab_control.h"
#include "row.h"
#include "grid.h"
#include "view.h"
#include "overlay.h"
#include "edit.h"
#include "column.h"
#include "app_bar.h"
#include "grid_item.h"
#include "combo_box.h"
#include "combo_box_item.h"
#include "tab_button_group.h"
#include "spin_box.h"
#include "digit_clock.h"
#endif /*AWTK_LITE*/
#endif /*AWTK_NOGUI*/

ret_t tk_widgets_init(void)
{
  widget_factory_t *f = widget_factory();

  FACTORY_TABLE_BEGIN(s_basic_widgets)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_DIALOG, dialog_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_NORMAL_WINDOW, window_create)
#ifndef AWTK_NOGUI
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_DIALOG_TITLE, dialog_title_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_DIALOG_CLIENT, dialog_client_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_ICON, icon_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_IMAGE, image_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_BUTTON, button_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_LABEL, label_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_PROGRESS_BAR, progress_bar_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SLIDER, slider_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_CHECK_BUTTON, check_button_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_RADIO_BUTTON, check_button_create_radio)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_PAGES, pages_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_BUTTON_GROUP, button_group_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_POPUP, popup_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_COLOR_TILE, color_tile_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_CLIP_VIEW, clip_view_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_GROUP_BOX, group_box_create)
#ifndef AWTK_LITE
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SYSTEM_BAR, system_bar_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SYSTEM_BAR_BOTTOM, system_bar_bottom_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_CALIBRATION_WIN, calibration_win_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_VIEW, view_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_PAGE, view_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_OVERLAY, overlay_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_EDIT, edit_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_TAB_CONTROL, tab_control_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_TAB_BUTTON, tab_button_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_TAB_BUTTON_GROUP, tab_button_group_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SPIN_BOX, spin_box_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_DRAGGER, dragger_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_COMBO_BOX, combo_box_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_COMBO_BOX_ITEM, combo_box_item_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_GRID, grid_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_GRID_ITEM, grid_item_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_ROW, row_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_COLUMN, column_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_APP_BAR, app_bar_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_DIGIT_CLOCK, digit_clock_create)
#endif /*AWTK_LITE*/
#endif /**AWTK_NOGUI*/
  FACTORY_TABLE_END()

  return widget_factory_register_multi(f, s_basic_widgets);
}
