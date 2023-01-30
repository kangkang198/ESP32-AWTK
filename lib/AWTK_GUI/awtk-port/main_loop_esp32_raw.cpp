#include <Arduino.h>
#include "../awtk/src/base/idle.h"
#include "../awtk/src/base/timer.h"
#include "../awtk/src/tkc/platform.h"
#include "../awtk/src/base/main_loop.h"
#include "../awtk/src/base/event_queue.h"
#include "../awtk/src/base/font_manager.h"
#include "../awtk/src/lcd/lcd_mem_fragment.h"
#include "../awtk/src/main_loop/main_loop_simple.h"

#define KEY_LEFT (1)
#define KEY_RIGHT (2)

#define KEY1_PIN (5)
#define KEY2_PIN (21)

ret_t platform_disaptch_input(main_loop_t *l) { return RET_OK; }

// ����ʹ�õ���SPI�ӿڵ�С�ߴ���Ļ,ֻ��ʹ��Ƭ��ʽ��framebuffer����������Ļ
lcd_t *platform_create_lcd(wh_t w, wh_t h)
{
    return lcd_mem_fragment_create(w, h);
}

void Key_Init(void)
{
    pinMode(KEY1_PIN, PULLUP);
    pinMode(KEY2_PIN, PULLUP);
}

uint8_t Key_Scan(void)
{
    static uint16_t key_up = 1; // �����ɿ���־
    if (key_up == 1 && (digitalRead(KEY1_PIN) == LOW || digitalRead(KEY2_PIN) == LOW))
    {
        delay(10); // ȥ����
        key_up = 0;
        if (digitalRead(KEY1_PIN) == LOW)
            return TK_KEY_LEFT;
        else if (digitalRead(KEY2_PIN) == LOW)
            return TK_KEY_RIGHT;
    }
    else if (digitalRead(KEY1_PIN) == HIGH && digitalRead(KEY2_PIN) == HIGH)
    {
        key_up = 1;
    }
    return 0; // �ް�������
}

void dispatch_input_events(void)
{
    bool is_press = 0; /* Ĭ�ϰ���Ϊ̧��״̬ */
    /* ��ȡ�����豸��Ϣ�������ʵ������ʵ�֣� */
    int key = Key_Scan();
    /* ��ȡ��ֵ�Ͱ���״̬��AWTK �ļ�ֵ��ο� awtk/src/base/keys.h */
    if (key)
    {
        main_loop_post_key_event(main_loop(), TRUE, key);
    }
    else
    {
        main_loop_post_key_event(main_loop(), FALSE, key);
    }
}

#include "../awtk/src/main_loop/main_loop_raw.inc"
