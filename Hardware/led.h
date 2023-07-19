#ifndef __LED_H__
#define __LED_H__

#include "ws2812b.h"

#define LED_NUM (1585)    // 32*8+50+135+1024+120

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


static const RGB_t red = {185, 0, 0};   //目前发现亮度太高好像会出现电流比较大导致信号线受到干扰的情况，所以把颜色调暗了一些
static const RGB_t blue = {0, 0, 185};
static const RGB_t off = {0, 0, 0};
static const RGB_t dim_red = {80, 0, 0};  //靶心的颜色
static const RGB_t dim_blue = {0, 0, 80};   //靶心的颜色



void LED_Init(LED_State_t state);
void LED_PackLightALLData(void);

void LED_PackRectangleData(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackStripData(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackFrameData1(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackFrameData2(LED_Leaf_Mode_t leafmode, RGB_t * dst);
void LED_PackTargetData(LED_Leaf_Mode_t leafmode, RGB_t * dst);



void LED_Update(void);

#endif
