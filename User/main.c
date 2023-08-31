#include "stm32f10x.h" // Device header
#include "led.h"
#include "infrared.h"
#include "Delay.h"
#include "rand.h"


int main(void)
{
	IR_Init();
	LED_Init(RedState);	
	rand_init();
	while (1)
	{
		Delay_ms(100);
		while(!ws2812b_IsReady());
		LED_Update();
	}
}
