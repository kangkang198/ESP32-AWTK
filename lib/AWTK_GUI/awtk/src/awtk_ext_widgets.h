/**
 * File:   awtk.h
 * Author: AWTK Develop Team
 * Brief:  awtk widgets
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
 * 2018-12-15 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef AWTK_EXT_WIDGETS_H
#define AWTK_EXT_WIDGETS_H

#include "ext_widgets/vpage/vpage.h"
#include "ext_widgets/switch/switch.h"
#include "ext_widgets/gauge/gauge.h"
#include "ext_widgets/gauge/gauge_pointer.h"
#include "ext_widgets/gif_image/gif_image.h"
#include "ext_widgets/svg_image/svg_image.h"
#include "ext_widgets/keyboard/keyboard.h"
#include "ext_widgets/keyboard/candidates.h"
#include "ext_widgets/keyboard/lang_indicator.h"
#include "base/widget_factory.h"
#include "ext_widgets/rich_text/rich_text.h"
#include "ext_widgets/rich_text/rich_text_view.h"
#include "ext_widgets/slide_menu/slide_menu.h"
#include "ext_widgets/image_value/image_value.h"
#include "ext_widgets/time_clock/time_clock.h"
#include "ext_widgets/scroll_view/list_item.h"
#include "ext_widgets/scroll_view/list_item_seperator.h"
#include "ext_widgets/scroll_view/list_view.h"
#include "ext_widgets/slide_view/slide_view.h"
#include "ext_widgets/slide_view/slide_indicator.h"
#include "ext_widgets/scroll_view/scroll_bar.h"
#include "ext_widgets/scroll_view/scroll_view.h"
#include "ext_widgets/scroll_view/list_view_h.h"
#include "ext_widgets/color_picker/color_picker.h"
#include "ext_widgets/canvas_widget/canvas_widget.h"
#include "ext_widgets/text_selector/text_selector.h"
#include "ext_widgets/color_picker/color_component.h"
#include "ext_widgets/progress_circle/progress_circle.h"
#include "ext_widgets/image_animation/image_animation.h"
#include "ext_widgets/mutable_image/mutable_image.h"
#include "ext_widgets/combo_box_ex/combo_box_ex.h"
#include "ext_widgets/scroll_label/hscroll_label.h"
#include "ext_widgets/mledit/line_number.h"
#include "ext_widgets/mledit/mledit.h"
#include "ext_widgets/features/draggable.h"
#include "ext_widgets/timer_widget/timer_widget.h"
#include "ext_widgets/serial_widget/serial_widget.h"

#if defined(WITH_FS_RES) || defined(WITH_FS)
#include "ext_widgets/file_browser/file_chooser.h"
#include "ext_widgets/file_browser/file_browser_view.h"
#endif /*WITH_FS*/

#include "ext_widgets/ext_widgets.h"

#endif /*AWTK_EXT_WIDGETS_H*/
