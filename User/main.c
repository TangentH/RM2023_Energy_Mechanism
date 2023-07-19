#include "stm32f10x.h" // Device header
#include "led.h"
#include "infrared.h"

uint8_t address;
uint8_t command;
RCC_ClocksTypeDef RCC_Clocks;		//查看这个结构体，即可获得系统时钟频率

int main(void)
{
	LED_Init(RedState);
	IR_Init();
	
	RCC_GetClocksFreq(&RCC_Clocks);
	while (1)
	{
		LED_Update();
		if(IR_GetDataFlag())
		{
			address = IR_GetAddress();
			command = IR_GetCommand();
		}

	}
}
