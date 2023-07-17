#include <stm32f10x.h>
#include "led.h"
#include "pattern.h"


void LED_Init(LED_State_t state)
{
    ws2812b_Init();
    LED_State = state;
}

void LED_PackData(LED_Leaf_Mode_t leafmode)
{

}

void LED_Update(void)
{

}
