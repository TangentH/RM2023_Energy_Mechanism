#include <stm32f10x.h>
#include "led.h"
#include "pattern.h"
#include "timer.h"
#include "rand.h"

uint8_t debug = 0;  //debug = 1模式下叶片不会2.5s切换，方便视觉对同一个叶片持续调参
uint8_t timeout = 0;       // 如果时间到了(2.5s)，timeout = 1，由定时器硬件中断修改
uint8_t refresh_rectangle = 0; // 如果刷新灯板（指的是让长方形灯板里面的箭头流动起来）时间到了，refresh_rectangle = 1，由定时器中硬件中断修改
uint8_t leaf_ring_value[5] = {0, 0, 0, 0, 0};   //取值范围0-4，分别对应2环，4环，6环，8环，10环
static LED_Leaf_Name_t current_Refresh_Leaf = LEAF_0; // 默认为第一片叶子,表示当前在刷新的扇叶

static RGB_t leds[LED_NUM];    //存放led点亮数据的地方
static RGB_t R_logo[64];   //存放R标的数据

LED_Leaf_Mode_t leafmode[5] = {LEAF_OFF, LEAF_OFF, LEAF_OFF, LEAF_OFF, LEAF_OFF}; //存放每片叶子当前的状态，默认为关闭
RGB_t current_color = {0, 0, 0}; //变量，表示这片叶子亮哪方的颜色（其实有后面的LED_State就可以判断了，但是为了减轻CPU负担，尽量在颜色判断上只在初始化的时候判断一次）


LED_State_t LED_State = RedState; //能量机关默认为红方模式
uint8_t currentLeafStruck = 0;· //当前叶子是否击中了
LED_Leaf_Name_t current_striking_leaf = LEAF_0; //当前击打的扇叶
uint8_t total_struck = 0; //总共击打的次数


void LED_Init(LED_State_t state)
{
    Timer_Init();
    ws2812b_Init();
    LED_State = state;
    current_color = (LED_State == RedState ? red : blue);

    //pack R logo data
    for (uint8_t i = 0; i < 64; i++)
    {
        R_logo[i] = current_color;
    }
}

void LED_PackLightALLData(void)
{
    current_color = dim_red;
    for (uint16_t i = 0; i < LED_NUM; i++)
    {
        leds[i] = current_color;
    }
}

void LED_PackRectangleData(LED_Leaf_Mode_t leafmode, RGB_t *dst)
{
    static uint8_t count = 4;
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint16_t i = 0; i < 256; i++)
        {
            dst[i] = off;
        }
        break;
    case LEAF_STRIKING:
        // 对于移动箭头，每移动4次就会回到初始位置
        if (refresh_rectangle == 1)
        {
            count--;
        }
        if (count > 0)
        {
            refresh_rectangle = 0; // 硬件置0(定时器中断)，软件清0
        }
        else if (count == 0)
        {
            count = 4;
            refresh_rectangle = 0;
        }
        for (uint16_t i = 0; i < 256; i++)
        {

            if ((i + count * 8 < 256) && RECTANGLE_ARROWS[i + count * 8])
            {
                dst[i] = current_color;
            }
            else if ((i + count * 8 >= 256) && RECTANGLE_ARROWS[i + count * 8 - 256])
            {
                dst[i] = current_color;
            }
            else
            {
                dst[i] = off;
            }
        }
        break;
    case LEAF_STRUCK:
        for (uint16_t i = 0; i < 256; i++)
        {
            dst[i] = current_color;
        }
        break;
    default:
        break;
    }
}

void LED_PackStripData(LED_Leaf_Mode_t leafmode, RGB_t *dst)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint8_t i = 0; i < 50; i++)
        {
            dst[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint8_t i = 0; i < 50; i++)
        {
            dst[i] = off;
        }
        break;
    case LEAF_STRUCK:
        for (uint8_t i = 0; i < 50; i++)
        {
            dst[i] = current_color;
        }
        break;
    default:
        break;
    }
}

void LED_PackFrameData1(LED_Leaf_Mode_t leafmode, RGB_t *dst)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint8_t i = 0; i < 135; i++)
        {
            dst[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint8_t i = 0; i < 135; i++)
        {
            dst[i] = current_color;
        }
        break;
    case LEAF_STRUCK:
        for (uint8_t i = 0; i < 135; i++)
        {
            dst[i] = current_color;
        }
        break;
    default:
        break;
    }
}

void LED_PackFrameData2(LED_Leaf_Mode_t leafmode, RGB_t *dst)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint8_t i = 0; i < 120; i++)
        {
            dst[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint8_t i = 0; i < 120; i++)
        {
            dst[i] = current_color;
        }
        break;
    case LEAF_STRUCK:
        for (uint8_t i = 0; i < 120; i++)
        {
            dst[i] = current_color;
        }
        break;
    default:
        break;
    }
}
void LED_PackTargetData(LED_Leaf_Mode_t leafmode, RGB_t *dst)
{
    static RGB_t target_color = {80, 0, 0};   //靶心的颜色,和dim_red一样
    if(LED_State == BlueState)
    {
        target_color = dim_blue;
    }
    else
    {
        target_color = dim_red;
    }
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint16_t i = 0; i < 1024; i++)
        {
            dst[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint16_t i = 0; i < 1024; i++)
        {
            if (TARGET_STRIKING[i])
            {
                dst[i] = target_color;
            }
            else
            {
                dst[i] = off;
            }
        }
        break;
    case LEAF_STRUCK:
        if (leaf_ring_value[current_Refresh_Leaf] == 0)
        {
            for (uint16_t i = 0; i < 1024; i++)
            {
                if (TARGET_STRUCK_ring2[i])
                {
                    dst[i] = target_color;
                }
                else
                {
                    dst[i] = off;
                }
            }
        }
        else if (leaf_ring_value[current_Refresh_Leaf] == 1)
        {
            for (uint16_t i = 0; i < 1024; i++)
            {
                if (TARGET_STRUCK_ring4[i])
                {
                    dst[i] = target_color;
                }
                else
                {
                    dst[i] = off;
                }
            }
        }
        else if (leaf_ring_value[current_Refresh_Leaf] == 2)
        {
            for (uint16_t i = 0; i < 1024; i++)
            {
                if (TARGET_STRUCK_ring6[i])
                {
                    dst[i] = target_color;
                }
                else
                {
                    dst[i] = off;
                }
            }
        }
        else if (leaf_ring_value[current_Refresh_Leaf] == 3)
        {
            for (uint16_t i = 0; i < 1024; i++)
            {
                if (TARGET_STRUCK_ring8[i])
                {
                    dst[i] = target_color;
                }
                else
                {
                    dst[i] = off;
                }
            }
        }
        else if (leaf_ring_value[current_Refresh_Leaf] == 4)
        {
            for (uint16_t i = 0; i < 1024; i++)
            {
                if (TARGET_STRUCK_ring10[i])
                {
                    dst[i] = target_color;
                }
                else
                {
                    dst[i] = off;
                }
            }
        }
        default:
            break;
        }
}

//以下函数会确定当下刷新时每片叶子是什么状态，并且会修改leftmode数组和leaf_ring_value数组
void check_LED_Status(void)
{
    static LED_Leaf_Name_t temp;    //用来存抽奖抽中的叶子
    //情况1：击中了
    if(timeout == 0 && currentLeafStruck == 1)
    {
        Timer_reset();
        currentLeafStruck = 0;
        total_struck++;
        leafmode[current_striking_leaf] = LEAF_STRUCK;
        while(total_struck < 5)    //如果不是全部打中，就一直抽到关闭的灯为止
        {
            temp = (LED_Leaf_Name_t) rand_get(5);
            //一直抽到没有点亮的扇叶为止
            if(leafmode[temp] == LEAF_OFF)
            {
                leafmode[temp] = LEAF_STRIKING;
                current_striking_leaf = temp;
                break;
            }
        }
    }
    //情况2：超时（不考虑打错的情况）
    else if(timeout == 1 && total_struck < 5)
    {
        timeout = 0;
        currentLeafStruck = 0;
        total_struck = 0;
        for(uint8_t i = 0; i < 5; i++)
        {
            leafmode[i] = LEAF_OFF;
            leaf_ring_value[i] = 0;
        }
        while(1)
        {
            temp = (LED_Leaf_Name_t) rand_get(5);
            if(temp != current_striking_leaf)  //避免连续抽到同一片扇叶
            {
                leafmode[temp] = LEAF_STRIKING;
                current_striking_leaf = temp;
                break;
            }
        }
    }
    //情况3：超时，但是5片叶子都击中了，需要复位
    else if(timeout == 1 && total_struck >= 5)
    {
        timeout = 0;
        total_struck = 0;
        for(uint8_t i = 0; i < 5; i++)
        {
            leafmode[i] = LEAF_OFF;
            leaf_ring_value[i] = 0;
        }
        current_striking_leaf = (LED_Leaf_Name_t) rand_get(5);
        leafmode[current_striking_leaf] = LEAF_STRIKING;
    }
    //情况4：未超时，则说明是保持状态，什么都不用做
}

// 以上的packData都是针对一片叶子的，现在希望连续刷新所有的叶子
void LED_Update(void)
{
    static RGB_t R_color = {255, 0, 0};
    if(LED_State == BlueState)
    {
        R_color = R_blue;
    }
    else
    {
        R_color = R_red;
    }
    while (!ws2812b_IsReady());
    check_LED_Status();
    while (!ws2812b_IsReady());     //跟据原作者的实例代码，需要先检查是否ready，再包装数据，而且每次发送前都要检查一遍否则可能出现数据串了的情况
    if (LED_State == DebugState)
    {
        LED_PackLightALLData();
    }
    else
    {
        LED_PackRectangleData(leafmode[current_Refresh_Leaf], leds);
        LED_PackStripData(leafmode[current_Refresh_Leaf], leds + 256);
        LED_PackFrameData1(leafmode[current_Refresh_Leaf], leds + 256 + 50);
        LED_PackTargetData(leafmode[current_Refresh_Leaf], leds + 256 + 50 + 135);
        LED_PackFrameData2(leafmode[current_Refresh_Leaf], leds + 256 + 50 + 135 + 1024);
    }
    switch (current_Refresh_Leaf)
    {
    case LEAF_0:
        ws2812b_H3_SendRGB(leds, LED_NUM);
        break;
    case LEAF_1:
        ws2812b_H4_SendRGB(leds, LED_NUM);
        break;
    case LEAF_2:
        ws2812b_H6_SendRGB(leds, LED_NUM);
        break;
    case LEAF_3:
        ws2812b_H7_SendRGB(leds, LED_NUM);
        break;
    case LEAF_4:
        ws2812b_H8_SendRGB(leds, LED_NUM);
        break;
    default:
        return;
    }
    if (current_Refresh_Leaf < LEAF_4)
    {
        current_Refresh_Leaf++;
    }
    else
    {
		while (!ws2812b_IsReady());     //跟据原作者的实例代码，需要先检查是否ready，再包装数据，而且每次发送前都要检查一遍否则可能出现数据串了的情况
            //pack R logo data
        for (uint8_t i = 0; i < 64; i++)
        {
            R_logo[i] = R_color;
        }
        ws2812b_SW1_SendRGB(R_logo, 64);    //R标单独发
        current_Refresh_Leaf = LEAF_0;
    }
}
