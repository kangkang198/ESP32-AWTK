#include "../awtk/src/tkc/mem.h"
#include "../awtk/src/base/timer.h"
#include "../awtk/src/lcd/lcd_mem_bgr565.h"
#include <Arduino.h>

// ��ֲ����Ҫȷ�� get_time_ms64 �������ص�ʱ���� 64λ��������������
// ƽ̨û���ṩ 64 λ������������Ҫ�û�����ͨ��ϵͳ�жϻ�Ӳ����ʱ��ʵ�֡����ǿ��
// �� get_time_ms64 �������� 32 λ��������GUI ��ʱ�����к���ܴ��ڼ���������պͲ���
// Ԥ������⡣
// ���⣬����ϵͳƽ̨��Ҳ����ͨ�� SysTick ��ʵ�����Ϻ�����ֻ���ʼ�� SyeTick ����
// �ڱ���ʱ��� awtk/src/platforms/raw/sys_tick_handler.c �ļ�����

/**
 * @method get_time_ms64
 * ��ȡ��ǰʱ��(����)��
 *
 * @return {uint64_t} �ɹ����ص�ǰʱ�䡣
 */
uint64_t get_time_ms64(void)
{
  return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

/**
 * @method sleep_ms
 *
 * ˯��ָ��ʱ�䡣
 *
 * @param {uint32_t} ms ˯��ʱ��(����)��
 *
 * @return {void} �ޡ�
 */
void sleep_ms(uint32_t ms)
{
  delay(ms);
}

/**
 * @method platform_prepare
 *
 * ƽ̨׼��������
 *
 * @return {ret_t} ����RET_OK��ʾ�ɹ��������ʾʧ�ܡ�
 */
/* �� AWTK ����һ���СΪ TK_MEM_SIZE ���ڴ� s_mem */
#define TK_MEM_SIZE 32 * 1024
ret_t platform_prepare(void)
{
  /* ����� HAS_STD_MALLOC ��ʹ��ϵͳ��׼ malloc ����������ʹ��ϵͳ�Դ���malloc */
#ifndef HAS_STD_MALLOC
  static uint8_t s_mem[TK_MEM_SIZE]; /* �˴��Ծ�̬����Ϊ�� */
  tk_mem_init(s_mem, sizeof(s_mem)); /* ��ʼ�� AWTK �ڴ������ */
#endif

  return RET_OK;
}
