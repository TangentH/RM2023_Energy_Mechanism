#ifndef __LED_H__
#define __LED_H__

#include "ws2812b.h"

#define LED_NUM (1586)    // 32*8+256+50+1024

typedef enum { LEAF_OFF, LEAF_STRIKING, LEAF_STRUCK, LED_TEST} LED_Leaf_Mode_t; //对于单片LED的状态，分为四种
//              关闭      打击中          击中          测试亮灯

typedef enum { NormalState, DebugState} LED_State_t; //对于整个LED的状态，分为两种

typedef enum{
    LEAF_0,
    LEAF_1,
    LEAF_2,
    LEAF_3,
    LEAF_4,
} LED_Leaf_Name_t;

static RGB_t leds[LED_NUM];    //存放led点亮数据的地方
static RGB_t red = {255, 0, 0};
static RGB_t blue = {0, 0, 255};
static RGB_t dim_red = {50, 0, 0};  //用于测试，颜色调暗了一些，防止太亮眼
static RGB_t led_off = {0, 0, 0};
static LED_State_t LED_State = NormalState; //默认为正常模式

void LED_Init(LED_State_t state);
void LED_PackData(LED_Leaf_Mode_t leafmode);
void LED_Update(void);

#endif
