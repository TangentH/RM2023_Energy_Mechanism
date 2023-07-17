#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "led.h"




int main(void)
{
	LED_Init(NormalState);

	while (1)
	{
		LED_Update();
	}
}
