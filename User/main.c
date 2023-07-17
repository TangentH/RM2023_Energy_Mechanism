#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "led.h"

RCC_ClocksTypeDef RCC_Clocks;

int main(void)
{
	 // 查看这个结构体，即可获得系统时钟频率
	ws2812b_Init();
	RCC_GetClocksFreq(&RCC_Clocks);

	while (1)
	{
		while (!ws2812b_IsReady())
			; // wait
		for (uint8_t i = 0; i < 120; i++)
		{
			leds[i] = red;
		}
		ws2812b_PA1_SendRGB(leds, 120);
		Delay_ms(50);

		while (!ws2812b_IsReady())
			; // wait
		ws2812b_PA2_SendRGB(leds, 120);
		Delay_ms(50);

		while (!ws2812b_IsReady())
			; // wait
		ws2812b_PA6_SendRGB(leds, 120);
		Delay_ms(50);

		while (!ws2812b_IsReady())
			; // wait
		ws2812b_PB1_SendRGB(leds, 120);
		Delay_ms(50);

		while (!ws2812b_IsReady())
			; // wait
		ws2812b_PA8_SendRGB(leds, 120);
		Delay_ms(50);
	}
}
