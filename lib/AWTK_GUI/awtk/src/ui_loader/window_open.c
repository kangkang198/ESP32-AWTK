﻿#include "../base/assets_manager.h"
#include "../base/window_manager.h"
#include "ui_loader_default.h"
#include "ui_builder_default.h"

static ret_t on_window_open(void *ctx, event_t *e)
{
  widget_t *to_close = WIDGET(ctx);
  widget_t *open = WIDGET(e->target);

  if (open != NULL && open->name != NULL)
  {
    log_debug("window %s open\n", open->name);
  }

  if (to_close != NULL)
  {
    log_debug("window %s close\n", to_close->name);
    window_manager_close_window_force(to_close->parent, to_close);
  }

  return RET_REMOVE;
}

static widget_t *window_open_with_name(const char *name, widget_t *to_close)
{
  widget_t *wm = window_manager();
  widget_t *win = widget_child(wm, name);

  if (win != NULL)
  {
    bool_t single_instance = widget_get_prop_bool(win, WIDGET_PROP_SINGLE_INSTANCE, FALSE);
    if (single_instance)
    {
      window_manager_close_window_force(wm, win);
    }
  }

  win = ui_loader_load_widget(name);
  if (win != NULL)
  {
    widget_on(win, EVT_WINDOW_OPEN, on_window_open, to_close);
  }

  return win;
}

typedef struct _ui_expr_info_t
{
  widget_t *to_close;
  widget_t *widget;
} ui_expr_info_t;

static ret_t ui_on_expr(void *ctx, const void *data)
{
  const char *name = (const char *)data;
  ui_expr_info_t *info = (ui_expr_info_t *)ctx;

  info->widget = window_open_with_name(name, info->to_close);

  return info->widget != NULL ? RET_OK : RET_FAIL;
}

static widget_t *window_open_with_expr(const char *exprs, widget_t *to_close)
{
  ui_expr_info_t info = {to_close, NULL};

  system_info_eval_exprs(system_info(), exprs, ui_on_expr, &info);

  return info.widget;
}

widget_t *window_open_and_close(const char *name, widget_t *to_close)
{
  return_value_if_fail(name != NULL, NULL);

  if (strchr(name, ',') != NULL || strchr(name, '$') != NULL)
  {
    return window_open_with_expr(name, to_close);
  }
  else
  {
    return window_open_with_name(name, to_close);
  }
}

widget_t *window_open(const char *name)
{
  return window_open_and_close(name, NULL);
}

widget_t *dialog_open(const char *name)
{
  return window_open(name);
}
