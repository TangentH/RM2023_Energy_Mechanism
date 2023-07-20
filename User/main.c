#include "stm32f10x.h" // Device header
#include "led.h"
#include "infrared.h"
#include "Delay.h"


int main(void)
{
	IR_Init();
	LED_Init(RedState);	
	
	while (1)
	{
		Delay_ms(80);
		LED_Update();
	}
}
