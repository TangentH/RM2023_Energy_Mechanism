#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "led.h"
#include "timer.h"
#include "rand.h"


int main(void)
{
	LED_Init(DebugState);
	
	while (1)
	{
		LED_Update();
	}
}
