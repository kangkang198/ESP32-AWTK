#include "../awtk/src/tkc/mem.h"
#include "../awtk/src/base/timer.h"
#include "../awtk/src/lcd/lcd_mem_bgr565.h"
#include <Arduino.h>

// 移植层需要确保 get_time_ms64 函数返回的时间是 64位毫秒计数器。如果
// 平台没有提供 64 位计数器，则需要用户自行通过系统中断或硬件定时器实现。如果强行
// 在 get_time_ms64 函数返回 32 位计数器，GUI 长时间运行后可能存在计数溢出风险和不可
// 预测的问题。
// 此外，在裸系统平台上也可以通过 SysTick 来实现以上函数，只需初始化 SyeTick 并且
// 在编译时添加 awtk/src/platforms/raw/sys_tick_handler.c 文件即可

/**
 * @method get_time_ms64
 * 获取当前时间(毫秒)。
 *
 * @return {uint64_t} 成功返回当前时间。
 */
uint64_t get_time_ms64(void)
{
  return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

/**
 * @method sleep_ms
 *
 * 睡眠指定时间。
 *
 * @param {uint32_t} ms 睡眠时间(毫秒)。
 *
 * @return {void} 无。
 */
void sleep_ms(uint32_t ms)
{
  delay(ms);
}

/**
 * @method platform_prepare
 *
 * 平台准备函数。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
/* 给 AWTK 开辟一块大小为 TK_MEM_SIZE 的内存 s_mem */
#define TK_MEM_SIZE 32 * 1024
ret_t platform_prepare(void)
{
  /* 定义宏 HAS_STD_MALLOC 将使用系统标准 malloc 函数，建议使用系统自带的malloc */
#ifndef HAS_STD_MALLOC
  static uint8_t s_mem[TK_MEM_SIZE]; /* 此处以静态数组为例 */
  tk_mem_init(s_mem, sizeof(s_mem)); /* 初始化 AWTK 内存管理器 */
#endif

  return RET_OK;
}
