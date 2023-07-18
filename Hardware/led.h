#ifndef __LED_H__
#define __LED_H__

#include "ws2812b.h"

#define LED_NUM (1586)    // 32*8+256+50+1024

typedef enum { LEAF_OFF, LEAF_STRIKING, LEAF_STRUCK} LED_Leaf_Mode_t; //对于单片LED的状态，分为四种
//              关闭      打击中          击中

typedef enum { RedState, BlueState, DebugState} LED_State_t; //对于整个LED的状态，分为两种

typedef enum{
    LEAF_0 = 0,
    LEAF_1,
    LEAF_2,
    LEAF_3,
    LEAF_4,
    LEAF_NUM
} LED_Leaf_Name_t;


static const RGB_t red = {255, 0, 0};
static const RGB_t blue = {0, 0, 255};
static const RGB_t off = {0, 0, 0};
static const RGB_t dim_red = {50, 0, 0};  //用于测试，颜色调暗了一些，防止太亮眼
static const RGB_t dim_blue = {0, 0, 50};   //用于测试，颜色调暗了一些，防止太亮眼



void LED_Init(LED_State_t state);
void LED_PackLightALLData(void);

void LED_PackRectangleData(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackStripData(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackFrameData1(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackFrameData2(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackTargetData(LED_Leaf_Mode_t leafmode, RGB_t * dst);



void LED_Update(void);

#endif
