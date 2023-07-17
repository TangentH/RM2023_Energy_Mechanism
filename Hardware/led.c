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

void LED_LightALL(void)
{
    current_color = (LED_State == RedState ? dim_red : dim_blue);
    for(uint16_t i = 0; i < LED_NUM; i++)
    {
        leds[i] = current_color;
    }
    //TODO:现在只点了一片叶子，还要点亮全部的叶子
}

void LED_PackRectangleData(LED_Leaf_Mode_t leafmode, RGB_t * leds)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint16_t i = 0; i < 256; i++)
        {
            leds[i] = off;
        }        
        break;
    case LEAF_STRIKING:
    //TODO:打击中的字模
    case LEAF_STRUCK:
        for (uint16_t i = 0; i < 256; i++)
        {
            leds[i] = current_color;
        }
        break;
    default:
        break;
    }

}

void LED_PackStripData(LED_Leaf_Mode_t leafmode, RGB_t * leds)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint8_t i = 0; i < 50; i++)
        {
            leds[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint8_t i = 0; i < 50; i++)
        {
            leds[i] = off;
        }
        break;
    case LEAF_STRUCK:
        for (uint8_t i = 0; i < 50; i++)
        {
            leds[i] = current_color;
        }
        break;
    default:
        break;
    }
}

void LED_PackFrameData1(LED_Leaf_Mode_t leafmode, RGB_t * leds)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint8_t i = 0; i < 136; i++)
        {
            leds[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint8_t i = 0; i < 136; i++)
        {
            leds[i] = current_color;
        }
        break;
    case LEAF_STRUCK:
        for (uint8_t i = 0; i < 136; i++)
        {
            leds[i] = current_color;
        }
        break;
    default:
        break;
    }
}

void LED_PackFrameData2(LED_Leaf_Mode_t leafmode, RGB_t * leds)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint8_t i = 0; i < 120; i++)
        {
            leds[i] = off;
        }        
        break;
    case LEAF_STRIKING:
        for (uint8_t i = 0; i < 120; i++)
        {
            leds[i] = current_color;
        }
        break;
    case LEAF_STRUCK:
        for (uint8_t i = 0; i < 120; i++)
        {
            leds[i] = current_color;
        }
        break;
    default:
        break;
    }
}
void LED_PackTargetData(LED_Leaf_Mode_t leafmode, RGB_t * leds)
{
    switch (leafmode)
    {
    case LEAF_OFF:
        for (uint16_t i = 0; i < 1024; i++)
        {
            leds[i] = off;
        }
        break;
    case LEAF_STRIKING:
        for (uint16_t i = 0; i < 1024; i++)
        {
            if(AIM_STRIKING[i])
            {
                leds[i] = current_color;
            }
            else
            {
                leds[i] = off;
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

    while(!ws2812b_IsReady());
    for (uint16_t i = 0; i < 32; i++)
    {
        leds[i] = red;
    }

}
