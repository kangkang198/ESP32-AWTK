#include "TFT_eSPI.h"
#include "../lib/AWTK_GUI/awtk/src/awtk.h"
#include "../lib/AWTK_GUI/awtk/demos/demo.h"
#include "../lib/AWTK_GUI/awtk/src/awtk_main.inc"

extern int gui_app_start(int lcd_w, int lcd_h);
extern TFT_eSPI tft;

void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);        // ∫·∆¡œ‘ æ
  tft.fillScreen(TFT_BLACK); // «Â∫⁄∆¡
}

void loop()
{
  gui_app_start(LCD_WIDTH, LCD_HEIGHT); // 160x80
}
