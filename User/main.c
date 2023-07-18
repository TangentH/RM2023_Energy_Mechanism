#include "stm32f10x.h" // Device header
#include "led.h"


int main(void)
{
	LED_Init(RedState);
	
	while (1)
	{
		LED_Update();
	}
}
