#include "stm32f10x.h"                  // Device header

uint32_t systime_ms = 0;    //两个作用：产生随机种子，生成随机数；计时2.5s, 计时区间是0ms-2500ms（循环计时）
uint16_t overflow_count = 0;
uint16_t cnt = 0;	//存CNT计数器的值

extern uint8_t timeout;
extern uint8_t refresh_rectangle;

void Timer_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_InternalClockConfig(TIM4);//不写也行，因为上电默认使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//相当于按照时钟频率来滤波
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//决定定时时间//////////////////////////////////////////////
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;//自动重装值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;//分频系数
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;//只有高级定时器才有，这里直接给0就可以了
	///////////////////////////////////////////////////////////
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	//定时器在时基单元初始化后会立刻产生一个更新事件（同时将中断标志位置1），这样就能使得自动重装值和分频系数立刻生效
	//但是副作用是如果不清除这个标志位，上电就会自动进入一次中断函数
	//所以下面在配置好NVIC前就要先把标志位清除
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//因为要提高TIM的精度（中断也一定程度上提供了时钟，所以提高它的中断优先级）
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4, ENABLE);
}

void Timer_reset(void)
{
    systime_ms = 0;
}

//每1ms产生一个中断
//可以更新随机种子
//可以计时2.5s
//还可以在扇叶被击打时产生动画的刷新时间
void TIM4_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		systime_ms++;
        if(systime_ms == 2499)  //2.5s计时到
        {
			Timer_reset();
            timeout = 1;
        }
		if(systime_ms % 100 == 0)   //每100ms刷新一次灯板
		{
			refresh_rectangle = 1;
		}
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
	overflow_count++;
}

//以下函数是用来给红外遥控提供时序计时用的（本来想用TIM5的，结果发现stm32f103c8t6没有TIM5）
void TIM4_StartCounter(void)
{
	overflow_count = 0;
	cnt = TIM_GetCounter(TIM4);
}

//跟据溢出次数和计数器的值计算出时间，单位是微秒
uint16_t TIM4_GetCounter(void)
{
	return overflow_count * 1000 + TIM_GetCounter(TIM4) - cnt;
}

