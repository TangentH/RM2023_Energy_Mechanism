// The MIT License (MIT)
//
// Copyright (c) 2015 Aleksandr Aleshin <silencer@quadrius.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __WS2812B_CONF_H
#define __WS2812B_CONF_H

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_dma.h>
#include <misc.h>

#define WS2812B_USE_GAMMA_CORRECTION
#define WS2812B_USE_PRECALCULATED_GAMMA_TABLE

#define WS2812B_BUFFER_SIZE     60
#define WS2812B_START_SIZE      2
//由于将原代码中的单通道输出改成了五通道输出，以下的宏定义取消
// #define WS2812B_APB1_RCC        RCC_APB1Periph_TIM2
// #define WS2812B_APB2_RCC        RCC_APB2Periph_GPIOA

// #define WS2812B_AHB_RCC         RCC_AHBPeriph_DMA1
//注意：GPIO引脚需要和TIM的输出比较通道对应，因为使用的是PWM输出
// #define WS2812B_GPIO            GPIOA
// #define WS2812B_GPIO_PIN        GPIO_Pin_1

// #define WS2812B_TIM             TIM2
// #define WS2812B_TIM_OCINIT      TIM_OC2Init
// #define WS2812B_TIM_OCPRELOAD   TIM_OC2PreloadConfig
// #define WS2812B_TIM_DMA_CC      TIM_DMA_CC2
// #define WS2812B_TIM_DMA_CCR     (WS2812B_TIM->CCR2)
//注意下面的DMA通道配置也要和TIM的输出比较通道对应（比如TIM4的通道1对应DMA1的通道1）
// #define WS2812B_DMA             DMA1
// #define WS2812B_DMA_CHANNEL     DMA1_Channel7
// #define WS2812B_DMA_IRQ         DMA1_Channel7_IRQn

// #define WS2812B_DMA_HANDLER     DMA1_Channel7_IRQHandler
// #define WS2812B_DMA_IT_TC       DMA1_IT_TC7 //DMA传输完成中断
// #define WS2812B_DMA_IT_HT       DMA1_IT_HT7 //DMA传输一半中断

#define WS2812B_IRQ_PRIO        3
#define WS2812B_IRQ_SUBPRIO     3

#define WS2812B_FREQUENCY       24000000
#define WS2812B_PERIOD          30

#define WS2812B_PULSE_HIGH      21
#define WS2812B_PULSE_LOW       9

#endif //__WS2812B_CONF_H
