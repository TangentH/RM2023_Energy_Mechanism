#include <stm32f10x.h>
#include "led.h"
#include "pattern.h"
#include "timer.h"

uint8_t change_leaf = 0;       // 如果时间到了(2.5s)或者击中了，change_leaf = 1，由定时器硬件中断修改
uint8_t refresh_rectangle = 0; // 如果刷新灯板（指的是让长方形灯板里面的箭头流动起来）时间到了，refresh_rectangle = 1，由定时器中硬件中断修改
uint8_t leaf_ring_value[5] = {4, 0, 0, 0, 0};   //取值范围0-4，分别对应2环，4环，6环，8环，10环
static LED_Leaf_Name_t current_Refresh_Leaf = LEAF_0; // 默认为第一片叶子,表示当前在刷新的扇叶

void LED_Init(LED_State_t state)
{
    Timer_Init();
    ws2812b_Init();
    LED_State = state;
    current_color = (LED_State == RedState ? red : blue);
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
    static uint8_t count = 0;
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
            count++;
        }
        if (count < 4)
        {
            refresh_rectangle = 0; // 硬件置0(定时器中断)，软件清0
        }
        else if (count == 4)
        {
            count = 0;
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
        for (uint8_t i = 0; i < 136; i++)
        {
            dst[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint8_t i = 0; i < 136; i++)
        {
            dst[i] = current_color;
        }
        break;
    case LEAF_STRUCK:
        for (uint8_t i = 0; i < 136; i++)
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
                dst[i] = current_color;
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
                    dst[i] = current_color;
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
                    dst[i] = current_color;
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
                    dst[i] = current_color;
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
                    dst[i] = current_color;
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
                    dst[i] = current_color;
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

// 以上的packData都是针对一片叶子的，现在希望连续刷新所有的叶子
void LED_Update(void)
{
    while (!ws2812b_IsReady());     //跟据原作者的实例代码，需要先检查是否ready，再包装数据，否则可能出现数据串了的情况
    if (LED_State == DebugState)
    {
        LED_PackLightALLData();
    }
    else
    {
        // LED_PackRectangleData(leafmode[current_Refresh_Leaf], leds);
        // LED_PackStripData(leafmode[current_Refresh_Leaf], leds + 256);
        // LED_PackFrameData1(leafmode[current_Refresh_Leaf], leds + 256 + 50);
        LED_PackTargetData(leafmode[current_Refresh_Leaf], leds);
        // LED_PackFrameData2(leafmode[current_Refresh_Leaf], leds + 256 + 50 + 136);
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
        current_Refresh_Leaf = LEAF_0;
    }
}
