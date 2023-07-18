#include <stm32f10x.h>
#include "led.h"
#include "pattern.h"

uint8_t change_leaf = 0;    //如果时间到了(2.5s)或者击中了，change_leaf = 1

void LED_Init(LED_State_t state)
{
    ws2812b_Init();
    LED_State = state;
    current_color = (LED_State == RedState ? red : blue);
}

void LED_PackLightALLData(void)
{
    current_color = dim_red;
    for(uint16_t i = 0; i < LED_NUM; i++)
    {
        leds[i] = current_color;
    }
}

void LED_PackRectangleData(LED_Leaf_Mode_t leafmode, RGB_t * dst)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint16_t i = 0; i < 256; i++)
        {
            dst[i] = off;
        }        
        break;
    case LEAF_STRIKING:
    //TODO:打击中的字模
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

void LED_PackStripData(LED_Leaf_Mode_t leafmode, RGB_t * dst)
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

void LED_PackFrameData1(LED_Leaf_Mode_t leafmode, RGB_t * dst)
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

void LED_PackFrameData2(LED_Leaf_Mode_t leafmode, RGB_t * dst)
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
void LED_PackTargetData(LED_Leaf_Mode_t leafmode, RGB_t * dst)
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
            if(AIM_STRIKING[i])
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
        //TODO：击中效果的字模
    default:
        break;
    }
}

//以上的packData都是针对一片叶子的，现在希望连续刷新所有的叶子
void LED_Update(void)
{
	while (!ws2812b_IsReady());
    static LED_Leaf_Name_t current_Refresh_Leaf = LEAF_0; //默认为第一片叶子,表示当前在刷新的扇叶
    if(LED_State == DebugState)
    {
        LED_PackLightALLData();
    }
    else
    {
        LED_PackRectangleData(leafmode[current_Refresh_Leaf], leds);
        LED_PackStripData(leafmode[current_Refresh_Leaf], leds + 256);
        LED_PackFrameData1(leafmode[current_Refresh_Leaf], leds + 256 + 50);
        LED_PackTargetData(leafmode[current_Refresh_Leaf], leds + 256 + 50 + 136);
        LED_PackFrameData2(leafmode[current_Refresh_Leaf], leds + 256 + 50 + 136 + 1024);
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
    if(current_Refresh_Leaf < LEAF_4)
    {
        current_Refresh_Leaf++;
    }
    else
    {
        current_Refresh_Leaf = LEAF_0;
    }
}
