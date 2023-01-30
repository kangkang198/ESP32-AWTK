# ESP32 移植 AWTK库(嵌入式GUI库)

## 一、 前言：

### 最近了解到有一个叫AWTK的嵌入式GUI库，是ZLG(周立功)开发的一个开源免费可商用的嵌入式GUI库。手头上刚好有一个自己最近设计的ESP32测试板，型号是ESP32_WROOM_32，和一个0.96寸80x160的TFT屏，就想着能不能把这个AWTK库移植到ESP32上玩一下。于是，我又开始给自己挖坑了。

## 二、移植资料准备：

### 这次移植中，用的开发框架不是ESP-IDF，而是自己比较喜欢的ESP32 Arduino框架，开发平台用的是VScode+Platformio插件进行开发。

### 首先，当然是先要到github官网下载awtk库，链接如下：

[AWTK库官网下载链接](https://github.com/zlgopen/awtk)

### 下载到awtk库后，只保留3rd、demos、res、src这4个文件夹,其中src和3rd文件夹中是awtk的核心代码，demos和res文件夹是为了移植后用于测试的demo需要用到的文件。

### 然后新建一个awtk文件夹，把这4个文件夹都拷贝到awtk文件夹中。然后在新建一个awtk-port的文件夹，用来存放移植需要用户修改的文件，最后在新建一个AWTK_GUI文件夹，把a**wtk文件夹和awtk-port文件夹都放到AWTK_GUI文件夹中，作为platformio的第三方扩展库放到lib文件夹中。如下图所示。

### 在移植过程中，自己也发现很多问题，比如说awtk库编译时找不到头文件，后面自己通过翻看awtk库的文件才发现里面的头文件路径有些问题，就是有些文件中对头文件的路径表述不清，有一些是相对路径，但有些又不是。自己也是花了很长时间把所有文件中的头文件路径都改成相对路径，同时也删除和注释很多多余的文件，才最终编译成功的。

## 三、移植步骤：

### 1.平台初始化

### 平台初始化通常用于初始化平台相关的准备模块，比如AWTK的内存管理器。不过我初始化内存的方式是用系统自带的malloc来动态分配内存，没有是用静态数组来分配内存，代码如下：

```
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
```

## 2.实现时钟和睡眠的函数

### 时钟一般用来计算时间间隔，实现定时器和动画功能；睡眠一般用于 AWTK 主循环限制 GUI 帧率，代码如下：

```
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
```

### 3.LCD显示函数实现：

### 屏幕用的是TFT_eSPI库，TFT IC是ST7735,通信方式是用了SPI接口，屏幕点亮比较简单，在TFT_eSPI库中的User_Setup.h中把用到的IC的宏定义打开，修改SPI接口的引脚号即可，最后在main.c中初始化一下即可点亮。

### 其中为了让AWTK库可以调用点亮屏幕的接口函数，需要在TFT_eSPI库中的TFT_eSPI.cpp文件中添加以下两个函数，同时在TFT_eSPI.h中的类中声明这两个函数。这两个函数会在awtk-port文件夹中的lcd_esp32_raw.cpp被调用。

```//移植AWTK需要添加的两个函数
//移植AWTK需要在TFT_eSPI.cpp添加的两个函数
void TFT_eSPI::esp32_write_data_func(uint16_t dat)

{

  begin_tft_write();

  pushColor(dat);

  end_tft_write();

}

void TFT_eSPI::esp32_set_window_func(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)

{

  begin_tft_write();

  setWindow(xs, ys, xe, ye);

  end_tft_write();

}
```

## 注意：

### 由于本次使用的屏幕是SPI接口的，所以只能使用片段式的Framebuffer的方式点亮，该方式不支持矢量画布vgcanvans,所以如果使用了扩展控件的话，如用到了环形进度条(progress circle)或者开关(Switch),将会出现环形进度条不显示，开关的圆角矩形消失的现象。

## 四、 GUI的设计：

### GUI设计使用的是AWTK官方提供的AWTK_Designer来开发，用这个软件可以所见即所得设计GUI，还能模拟运行，非常方便。

## 五、GUI的部署到ESP32的方法：

### 只需把设计好的GUI先编译，再打包，当然模拟运行也可以打包，如下图所示：

### 打包好后就可以到AWTK_Designer的工程目录里把res、src两个文件夹拷贝出来。把src文件夹改名为demos文件夹，去分别替代自己ESP32工程目录里的res文件夹和demos文件夹，如下图所示：

### 其中src文件夹里的main.c文件需要改为demo.h，同时内容换成如下代码：

```
#ifndef DEMO_H
#define DEMO_H

BEGIN_C_DECLS

#include "../src/tkc/types_def.h"
#include "../res/assets.inc"

ret_t application_init();
ret_t application_exit();

END_C_DECLS
#endif /* DEMO_H */
```

### 当然,也可以换成其他名字的头文件，这个头文件主要是为了对外声明，application_init，application_exit两个函数。

## 六、效果展示

## 七、代码开源

### 为了方便大家更好的移植，这个ESP32移植AWTK的工程代码将会完全开源，当然这个工程代码也有不足，比如说外部输入设备还没有移植进去，大家可以继续看官网的AWTK移植文档去继续移植。

[AWTK官网文档链接](https://awtk.zlg.cn/docs)

[开源github仓库链接](https://github.com/kangkang198/ESP32-AWTK)
