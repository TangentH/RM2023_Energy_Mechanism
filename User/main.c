#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "ws2812b.h"

#define NUM_LEDS 1024

static RGB_t leds[NUM_LEDS];
static RGB_t led_red = {255, 0, 0};
static RGB_t led_blue = {0,0, 255};

int main(void)
{
	ws2812b_Init();

	while (1)
	{
		while (!ws2812b_IsReady()); // wait

		//
		// Fill leds buffer
		//
		for(int i = 0; i < NUM_LEDS; i++)
		{
			leds[i] = led_red;
		}
		


		ws2812b_SendRGB(leds, NUM_LEDS);
	}
}
