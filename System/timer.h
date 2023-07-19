#ifndef __TIMER_H__
#define __TIMER_H__


void Timer_Init(void);
void Timer_reset(void);
void TIM4_StartCounter(void);
uint16_t TIM4_GetCounter(void);

#endif
