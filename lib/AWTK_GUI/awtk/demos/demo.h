#ifndef DEMO_H
#define DEMO_H

// #define APP_LCD_ORIENTATION LCD_ORIENTATION_0
// #define APP_TYPE APP_SIMULATOR
// #define APP_NAME "mini_demo"

BEGIN_C_DECLS

#include "../src/tkc/types_def.h"
#include "../res/assets.inc"

ret_t application_init();
ret_t application_exit();

END_C_DECLS
#endif /* DEMO_H */
