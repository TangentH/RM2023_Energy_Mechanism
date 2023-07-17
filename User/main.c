#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "led.h"
#include "timer.h"
#include "rand.h"


extern uint32_t systime_ms;
uint8_t a=0;

int main(void)
{
	Timer_Init();

	while (1)
	{
		a = rand_get(5);
		Delay_ms(500);
	}
}
