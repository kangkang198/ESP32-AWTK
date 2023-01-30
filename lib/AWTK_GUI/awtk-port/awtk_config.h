
/**
 * File:   awtk_config.h
 * Author: AWTK Develop Team
 * Brief:  config
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
 * 2018-09-12 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef AWTK_CONFIG_H
#define AWTK_CONFIG_H

/**
 * Ƕ��ʽϵͳ���Լ���main����ʱ���붨�屾�ꡣ
 *
 * #define USE_GUI_MAIN 1
 */
#define USE_GUI_MAIN 1
/**
 * �����Ҫ֧��Ԥ�Ƚ����λͼ���壬�붨�屾�ꡣһ��ֻ��RAM��Сʱ�������ñ��ꡣ
 * #define WITH_BITMAP_FONT 1
 */

/**
 * ���֧��png/jpegͼƬ���붨�屾��
 *
 * #define WITH_STB_IMAGE 1
 */
#define WITH_STB_IMAGE 1
/**
 * �����stb֧��Truetype���壬�붨�屾��
 *
 * #define WITH_STB_FONT 1
 */
#define WITH_STB_FONT 1
/**
 * �����freetype֧��Truetype���壬�붨�屾�ꡣ
 *
 * #define WITH_FT_FONT 1
 */

/**
 * ���֧�ִ��ļ�ϵͳ������Դ���붨�屾��
 *
 * #define WITH_FS_RES 1
 */

/**
 * ���������flash�У�����Դ���ļ�ϵͳ���붨�屾��ָ����Դ���ڵ�·����
 *
 * #define APP_RES_ROOT "0://awtk/"
 *
 */

/**
 * ������屾�꣬ʹ�ñ�׼��UNICODE�����㷨��������Դ��Ϊ���ޣ��붨�屾�ꡣ
 *
 * #define WITH_UNICODE_BREAK 1
 */
#define WITH_UNICODE_BREAK 1
/**
 * ������屾�꣬��ͼƬ�����BGRA8888��ʽ����������RGBA8888�ĸ�ʽ��
 * ��Ӳ����2D������ҪBGRA��ʽʱ�������ñ��ꡣ
 *
 * #define WITH_BITMAP_BGRA 1
 */

/**
 * ������屾�꣬����͸����PNGͼƬ�����BGR565��ʽ�����鶨�塣
 * �����LCD�ĸ�ʽ����һ�£����Դ����������ܡ�
 * ���û�ж��� WITH_BITMAP_BGR565 �� WITH_BITMAP_RGB565 �꣬Ĭ�Ͻ���Ϊ32λɫ
 *
 * #define WITH_BITMAP_BGR565 1
 */
#define WITH_BITMAP_BGR565 1
/**
 * ����������뷨���붨�屾��
 *
 * #define WITH_NULL_IM 1
 */
#define WITH_NULL_IM 1
/**
 * ����б�׼��malloc/free/calloc�Ⱥ������붨�屾��
 *
 * #define HAS_STD_MALLOC 1
 */
#define HAS_STD_MALLOC 1
/**
 * ����б�׼��fopen/fclose�Ⱥ������붨�屾��
 *
 * #define HAS_STDIO 1
 */

/**
 * ����б�׼��pthread�Ⱥ������붨�屾��
 *
 * #define HAS_PTHREAD 1
 */

/**
 * ������Ż��汾��memcpy�������붨�屾��
 *
 * #define HAS_FAST_MEMCPY 1
 */

/**
 * �������wcsxxx֮��ĺ���û�ж���ʱ���붨��ú�
 *
 * #define WITH_WCSXXX 1
 */
#define WITH_WCSXXX 1
/**
 * �������STM32 G2DӲ�����٣��붨�屾��
 *
 * #define WITH_STM32_G2D 1
 */

/**
 * �������NXP PXPӲ�����٣��붨�屾��
 *
 * #define WITH_PXP_G2D 1
 */

/**
 * ��û��GPUʱ���������agge��Ϊnanovg�ĺ��(��agg��Ϊ��ˣ�С���죬ͼ�������Բ�)���붨�屾�ꡣ
 *
 * #define WITH_NANOVG_AGGE 1
 */
#define WITH_NANOVG_AGGE 1
/**
 * ��û��GPUʱ���������agg��Ϊnanovg�ĺ��(��agge��Ϊ��ˣ�������ͼ��������)���붨�屾�ꡣ
 * ע�⣺agg����GPLЭ�鿪Դ��
 *
 * #define WITH_NANOVG_AGG 1
 */

/**
 * ����������ָ�룬�붨�屾��
 *
 * #define ENABLE_CURSOR 1
 */

/**
 * ���ڵͶ�ƽ̨�������ʹ�ÿؼ��������붨�屾�ꡣ
 *
 * #define WITHOUT_WIDGET_ANIMATORS 1
 */

/**
 * ���ڵͶ�ƽ̨�������ʹ�ô��ڶ������붨�屾�ꡣ
 *
 * #define WITHOUT_WINDOW_ANIMATORS 1
 */
#define WITHOUT_WINDOW_ANIMATORS 1
/**
 * ���ڵͶ�ƽ̨�������ʹ�öԻ���������ԣ��붨�屾�ꡣ
 *
 * #define WITHOUT_DIALOG_HIGHLIGHTER 1
 */
#define WITHOUT_DIALOG_HIGHLIGHTER 1
/**
 * ���ڵͶ�ƽ̨�������ʹ����չ�ؼ����붨�屾�ꡣ
 *
 * #define WITHOUT_EXT_WIDGETS 1
 */

/**
 * ���ڵͶ�ƽ̨������ڴ治�����ṩ������FrameBuffer���붨�屾�����þֲ�FrameBuffer���ɴ���������Ⱦ���ܡ�(��λ�����ظ���)
 *
 * #define FRAGMENT_FRAME_BUFFER_SIZE 32 * 1024
 */
#define FRAGMENT_FRAME_BUFFER_SIZE 32 * 1024

/**
 * �������뷨���������������빦�ܣ��붨�屾�ꡣ
 *
 * #define WITHOUT_SUGGEST_WORDS 1
 */

/**
 * �����Ҫ��zip�ļ��м�����Դ���붨�屾�ꡣ
 *
 * #define WITH_ASSET_LOADER_ZIP 1
 */

/**
 * ����ֻ��512K flash��ƽ̨������LCD��ʽ��BGR565�����ϣ����һ���Ż��ռ䣬ȥ�������bitmap��ʽ֧�ִ��롣�붨�屾�ꡣ
 * ����LCD��ʽ�����������޸ģ�src/blend/soft_g2d.c ������Ҫ�ĸ�ʽ���ɡ�
 *
 * #define LCD_BGR565_LITE 1
 */

/**
 * ���ϣ��֧������˫���Ű��㷨(�簢��������)���붨�屾�ꡣ
 *
 * #define WITH_TEXT_BIDI 1
 *
 */

/**
 * �����Զ�������� canvas���������ʹ������Ļ����Ļ�����Ҫ����ú��������������� canvas ����
 *
 * #define WITH_CANVAS_OFFLINE_CUSTION 1
 */
#define WITH_CANVAS_OFFLINE_CUSTION 1
/**
 * ����͸��ɫ������ˢ�»��ƣ�һ��ʹ���ڶ�ͼ���͸������ʹ��
 *
 * #define WITH_LCD_CLEAR_ALPHA 1
 */

/**
 * ���֧�ֶ�鲻�������ڴ�飬�붨���ڴ�����Ŀ��
 *
 * #define TK_MAX_MEM_BLOCK_NR 4
 */

/**
 * ���ڿ��ƴ��ڶ����Ƿ�ʹ�û��档��������Խ����ڴ�����(����2��framebuffer��С���ڴ�)������ڴ��ȱ��������������Կ�����
 *  1. �����
 *  2. CPU�ٶȿ�
 *
 * ��������ٶ����������ڴ��ȱ������رմ��ڶ�����
 *
 * ����������
 *  1.��֧�����Ŵ��ڶ�����
 *  2.��֧�ֶԻ���������ԡ�
 *
 * #define WITHOUT_WINDOW_ANIMATOR_CACHE 1
 */

/**
 * �����Ҫ�����ļ�����ʹ��data_reader/data_writer���붨�屾�ꡣ
 *
 * #define WITH_DATA_READER_WRITER 1
 */

/**
 * ���ڵͶ�ƽ̨�������ʹ�� fscript ģ�飬�붨�屾�ꡣ
 *
 * #define WITHOUT_FSCRIPT 1
 */
#define WITHOUT_FSCRIPT 1
/**
 * ���ڼ������(3keys/5keys)�����ϣ������״̬���ֲ�ͬ�����Ч�����붨�屾�ꡣ
 *
 * #define WITH_STATE_ACTIVATED 1
 */

/**
 * ������Ч�� lcd ��ת����ģ�飨�����������Ļ��תЧ����
 * ע�⣺
 * 1����Ҫ���±�����Դ�������ͼƬԤ�ȴ�����ת���λͼ���ݡ�(�����Ҫʵ������ʱ��̬�ı���ת�Ļ���ͼƬ��Դ��ֻ��Ϊδ��ת)
 * 2��lcd �������Ҫ֧�֡�
 * 3��vg ���������Ҫ֧�֡�
 * 4��g2d ��������ȡλͼ��ʵ��������Ҫͨ�� bitmap_get_physical_XXXX ��������ȡ��
 * 5����Ҫ�û��ڳ���ʼ֮ǰ���� tk_enable_fast_lcd_portrait ���������ù��ܡ�
 *
 * #define WITH_FAST_LCD_PORTRAIT 1
 */

#endif /*AWTK_CONFIG_H*/
