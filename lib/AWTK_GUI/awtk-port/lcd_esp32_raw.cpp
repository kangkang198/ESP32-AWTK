#include "TFT_eSPI.h"
#include "../awtk/src/tkc/mem.h"
#include "../awtk/src/lcd/lcd_mem_fragment.h"

typedef uint16_t pixel_t;

TFT_eSPI tft = TFT_eSPI();

#define LCD_FORMAT BITMAP_FMT_BGR565
#define pixel_from_rgb(r, g, b) \
  ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define pixel_from_rgba(r, g, b, a) \
  ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define pixel_to_rgba(p)                                                         \
  {                                                                              \
    (0xff & ((p >> 11) << 3)), (0xff & ((p >> 5) << 2)), (0xff & (p << 3)), 0xff \
  }

extern void write_data_func(uint16_t dat);
extern void set_window_func(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

void write_data_func(uint16_t dat)
{
  tft.esp32_write_data_func(dat);
}
void set_window_func(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
  tft.esp32_set_window_func(x_start, y_start, x_end, y_end);
}

#include "../awtk/src/base/pixel.h"
#include "../awtk/src/blend/pixel_ops.inc"
#include "../awtk/src/lcd/lcd_mem_fragment.inc"
