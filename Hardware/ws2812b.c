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

#include <stdint.h>
#include <string.h>

#include "bitmap.h"

#include "ws2812b.h"
#include "ws2812b_conf.h"

//------------------------------------------------------------
// Internal
//------------------------------------------------------------

#define MIN(a, b)   ({ typeof(a) a1 = a; typeof(b) b1 = b; a1 < b1 ? a1 : b1; })

#if defined(__ICCARM__)
__packed struct PWM
#else
struct __attribute__((packed)) PWM
#endif
{
    uint16_t g[8], r[8], b[8];
};

typedef struct PWM PWM_t;
typedef void (SrcFilter_t)(void **, PWM_t **, unsigned *, unsigned);

#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
static const uint8_t LEDGammaTable[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
    10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21,
    22, 23, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38,
    38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84,
    85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 97, 98, 99, 100, 102, 103, 104, 105, 107, 108, 109, 111,
    112, 113, 115, 116, 117, 119, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137,
    139, 140, 142, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166,
    168, 170, 171, 173, 175, 176, 178, 180, 181, 183, 185, 186, 188, 190, 192, 193, 195, 197, 199,
    200, 202, 204, 206, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224, 226, 228, 230, 232, 233,
    235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255 };
#endif
#endif

static inline uint8_t LEDGamma(uint8_t v)
{
#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
    return LEDGammaTable[v];
#else
    return (v * v + v) >> 8;
#endif
#else
    return v;
#endif
}

static volatile int DMA1_CH7Busy;
static volatile int DMA1_CH1Busy;
static volatile int DMA1_CH6Busy;
static volatile int DMA1_CH3Busy;
static volatile int DMA1_CH2Busy;


static PWM_t DMABuffer[WS2812B_BUFFER_SIZE];

static SrcFilter_t *DMAFilter;
static void *DMASrc;
static unsigned DMACount;

static void SrcFilterNull(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
    memset(*pwm, 0, size * sizeof(PWM_t));
    *pwm += size;
}

static void RGB2PWM(RGB_t *rgb, PWM_t *pwm)
{
    uint8_t r = LEDGamma(rgb->r);
    uint8_t g = LEDGamma(rgb->g);
    uint8_t b = LEDGamma(rgb->b);

    uint8_t mask = 128;

    int i;
    for (i = 0; i < 8; i++)
    {
        pwm->r[i] = r & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
        pwm->g[i] = g & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
        pwm->b[i] = b & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;

        mask >>= 1;
    }
}

static void SrcFilterRGB(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
    RGB_t *rgb = *src;
    PWM_t *p = *pwm;

    *count -= size;

    while (size--)
    {
        RGB2PWM(rgb++, p++);
    }

    *src = rgb;
    *pwm = p;
}

static void SrcFilterHSV(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
    HSV_t *hsv = *src;
    PWM_t *p = *pwm;

    *count -= size;

    while (size--)
    {
        RGB_t rgb;

        HSV2RGB(hsv++, &rgb);
        RGB2PWM(&rgb, p++);
    }

    *src = hsv;
    *pwm = p;
}
//------------------------------------------------------------
static void DMA1_CH7Send(SrcFilter_t *filter, void *src, unsigned count)
{
    if (!DMA1_CH7Busy)
    {
        DMA1_CH7Busy = 1;

        DMAFilter = filter;
        DMASrc = src;
        DMACount = count;

        PWM_t *pwm = DMABuffer;     //TODO：迟点要确定一下这个DMABuffer需不需要给每个通道都分配一个
        PWM_t *end = &DMABuffer[WS2812B_BUFFER_SIZE];

        // Start sequence
        SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        // Start transfer
        DMA_SetCurrDataCounter(DMA1_Channel7, sizeof(DMABuffer) / sizeof(uint16_t));

        TIM_Cmd(TIM2, ENABLE);
        DMA_Cmd(DMA1_Channel7, ENABLE);
    }
}

static void DMA1_CH7SendNext(PWM_t *pwm, PWM_t *end)
{
    if (!DMAFilter)
    {
        // Stop transfer
        TIM_Cmd(TIM2, DISABLE);
        DMA_Cmd(DMA1_Channel7, DISABLE);

        DMA1_CH7Busy = 0;
    }
    else if (!DMACount)
    {
        // Rest of buffer
        SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        DMAFilter = NULL;
    }
    else
    {
        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT7) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_HT7);
        DMA1_CH7SendNext(DMABuffer, &DMABuffer[WS2812B_BUFFER_SIZE / 2]);
    }

    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC7);
        DMA1_CH7SendNext(&DMABuffer[WS2812B_BUFFER_SIZE / 2], &DMABuffer[WS2812B_BUFFER_SIZE]);
    }
}

//------------------------------------------------------------
static void DMA1_CH1Send(SrcFilter_t *filter, void *src, unsigned count)
{
    if (!DMA1_CH1Busy)
    {
        DMA1_CH1Busy = 1;

        DMAFilter = filter;
        DMASrc = src;
        DMACount = count;

        PWM_t *pwm = DMABuffer;
        PWM_t *end = &DMABuffer[WS2812B_BUFFER_SIZE];

        // Start sequence
        SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        // Start transfer
        DMA_SetCurrDataCounter(DMA1_Channel1, sizeof(DMABuffer) / sizeof(uint16_t));

        TIM_Cmd(TIM2, ENABLE);
        DMA_Cmd(DMA1_Channel1, ENABLE);
    }
}

static void DMA1_CH1SendNext(PWM_t *pwm, PWM_t *end)
{
    if (!DMAFilter)
    {
        // Stop transfer
        TIM_Cmd(TIM2, DISABLE);
        DMA_Cmd(DMA1_Channel1, DISABLE);

        DMA1_CH1Busy = 0;
    }
    else if (!DMACount)
    {
        // Rest of buffer
        SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        DMAFilter = NULL;
    }
    else
    {
        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT1) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_HT1);
        DMA1_CH1SendNext(DMABuffer, &DMABuffer[WS2812B_BUFFER_SIZE / 2]);
    }

    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        DMA1_CH1SendNext(&DMABuffer[WS2812B_BUFFER_SIZE / 2], &DMABuffer[WS2812B_BUFFER_SIZE]);
    }
}

//------------------------------------------------------------
static void DMA1_CH6Send(SrcFilter_t *filter, void *src, unsigned count)
{
    if (!DMA1_CH6Busy)
    {
        DMA1_CH6Busy = 1;

        DMAFilter = filter;
        DMASrc = src;
        DMACount = count;

        PWM_t *pwm = DMABuffer;
        PWM_t *end = &DMABuffer[WS2812B_BUFFER_SIZE];

        // Start sequence
        SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        // Start transfer
        DMA_SetCurrDataCounter(DMA1_Channel6, sizeof(DMABuffer) / sizeof(uint16_t));

        TIM_Cmd(TIM3, ENABLE);  //该到这里了
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
}

static void DMA1_CH6SendNext(PWM_t *pwm, PWM_t *end)
{
    if (!DMAFilter)
    {
        // Stop transfer
        TIM_Cmd(TIM3, DISABLE);
        DMA_Cmd(DMA1_Channel6, DISABLE);

        DMA1_CH6Busy = 0;
    }
    else if (!DMACount)
    {
        // Rest of buffer
        SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        DMAFilter = NULL;
    }
    else
    {
        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);
    }
}

void DMA1_Channel6_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT6) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_HT6);
        DMA1_CH6SendNext(DMABuffer, &DMABuffer[WS2812B_BUFFER_SIZE / 2]);
    }

    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        DMA1_CH6SendNext(&DMABuffer[WS2812B_BUFFER_SIZE / 2], &DMABuffer[WS2812B_BUFFER_SIZE]);
    }
}

//------------------------------------------------------------
static void DMA1_CH3Send(SrcFilter_t *filter, void *src, unsigned count)
{
    if (!DMA1_CH3Busy)
    {
        DMA1_CH3Busy = 1;

        DMAFilter = filter;
        DMASrc = src;
        DMACount = count;

        PWM_t *pwm = DMABuffer;
        PWM_t *end = &DMABuffer[WS2812B_BUFFER_SIZE];

        // Start sequence
        SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        // Start transfer
        DMA_SetCurrDataCounter(DMA1_Channel3, sizeof(DMABuffer) / sizeof(uint16_t));

        TIM_Cmd(TIM3, ENABLE);
        DMA_Cmd(DMA1_Channel3, ENABLE);
    }
}

static void DMA1_CH3SendNext(PWM_t *pwm, PWM_t *end)
{
    if (!DMAFilter)
    {
        // Stop transfer
        TIM_Cmd(TIM3, DISABLE);
        DMA_Cmd(DMA1_Channel3, DISABLE);

        DMA1_CH3Busy = 0;
    }
    else if (!DMACount)
    {
        // Rest of buffer
        SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        DMAFilter = NULL;
    }
    else
    {
        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT3) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_HT3);
        DMA1_CH3SendNext(DMABuffer, &DMABuffer[WS2812B_BUFFER_SIZE / 2]);
    }

    if (DMA_GetITStatus(DMA1_IT_TC3) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC3);
        DMA1_CH3SendNext(&DMABuffer[WS2812B_BUFFER_SIZE / 2], &DMABuffer[WS2812B_BUFFER_SIZE]);
    }
}

//------------------------------------------------------------
static void DMA1_CH2Send(SrcFilter_t *filter, void *src, unsigned count)
{
    if (!DMA1_CH2Busy)
    {
        DMA1_CH2Busy = 1;

        DMAFilter = filter;
        DMASrc = src;
        DMACount = count;

        PWM_t *pwm = DMABuffer;
        PWM_t *end = &DMABuffer[WS2812B_BUFFER_SIZE];

        // Start sequence
        SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        // Start transfer
        DMA_SetCurrDataCounter(DMA1_Channel2, sizeof(DMABuffer) / sizeof(uint16_t));

        TIM_Cmd(TIM1, ENABLE);
        TIM_CtrlPWMOutputs(TIM1,ENABLE);    //高级定时器主输出使能需要这一句
        DMA_Cmd(DMA1_Channel2, ENABLE);
    }
}

static void DMA1_CH2SendNext(PWM_t *pwm, PWM_t *end)
{
    if (!DMAFilter)
    {
        // Stop transfer
        TIM_Cmd(TIM1, DISABLE);
        TIM_CtrlPWMOutputs(TIM1,ENABLE);
        DMA_Cmd(DMA1_Channel2, DISABLE);

        DMA1_CH2Busy = 0;
    }
    else if (!DMACount)
    {
        // Rest of buffer
        SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        DMAFilter = NULL;
    }
    else
    {
        // RGB PWM data
        DMAFilter(&DMASrc, &pwm, &DMACount, MIN(DMACount, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);
    }
}

void DMA1_Channel2_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT2) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_HT2);
        DMA1_CH2SendNext(DMABuffer, &DMABuffer[WS2812B_BUFFER_SIZE / 2]);
    }

    if (DMA_GetITStatus(DMA1_IT_TC2) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC2);
        DMA1_CH2SendNext(&DMABuffer[WS2812B_BUFFER_SIZE / 2], &DMABuffer[WS2812B_BUFFER_SIZE]);
    }
}

//------------------------------------------------------------
// Interface
//------------------------------------------------------------

void ws2812b_Init(void)
{
    // Turn on peripheral clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);


    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // Initialize GPIO pin
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_6 | GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Initialize timer clock
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);

    TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

    
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    
    TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);


    // Initialize timer PWM
    TIM_OCInitTypeDef TIM_OCInitStruct;

    TIM_OCStructInit(&TIM_OCInitStruct);

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC2Init(TIM2, &TIM_OCInitStruct);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_OC3Init(TIM2, &TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // TIM_OC4Init(TIM2, &TIM_OCInitStruct);
    // TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    TIM_OC1Init(TIM3, &TIM_OCInitStruct);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    TIM_OC4Init(TIM3, &TIM_OCInitStruct);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    TIM_OC1Init(TIM1, &TIM_OCInitStruct);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // Initialize DMA channel
    DMA_InitTypeDef DMA_InitStruct;

    DMA_StructInit(&DMA_InitStruct);

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (TIM2->CCR2);
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer) / sizeof(uint16_t);
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel7, &DMA_InitStruct);

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (TIM2->CCR3);
    DMA_Init(DMA1_Channel1, &DMA_InitStruct);

    // DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (TIM2->CCR4);
    // DMA_Init(DMA1_Channel7, &DMA_InitStruct);

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (TIM3->CCR1);
    DMA_Init(DMA1_Channel6, &DMA_InitStruct);

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (TIM3->CCR4);
    DMA_Init(DMA1_Channel3, &DMA_InitStruct);

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (TIM1->CCR1);
    DMA_Init(DMA1_Channel2, &DMA_InitStruct);


    

    // Turn on timer DMA requests
    TIM_DMACmd(TIM2, TIM_DMA_CC2, ENABLE);
    TIM_DMACmd(TIM2, TIM_DMA_CC3, ENABLE);
    // TIM_DMACmd(TIM2, TIM_DMA_CC4, ENABLE);
    TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
    TIM_DMACmd(TIM3, TIM_DMA_CC4, ENABLE);
    TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);


    // Initialize DMA interrupt
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_Init(&NVIC_InitStruct);

    // Enable DMA interrupt
    DMA_ITConfig(DMA1_Channel7, DMA_IT_HT | DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_HT | DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel6, DMA_IT_HT | DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_HT | DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel2, DMA_IT_HT | DMA_IT_TC, ENABLE);
}

inline int ws2812b_IsReady(void)
{
    return !(DMA1_CH1Busy || DMA1_CH7Busy || DMA1_CH6Busy || DMA1_CH3Busy || DMA1_CH2Busy);
}

//------------------------------------------------------------
void ws2812b_PA1_SendRGB(RGB_t *rgb, unsigned count)
{
    DMA1_CH7Send(&SrcFilterRGB, rgb, count);
}

void ws2812b_PA1_SendHSV(HSV_t *hsv, unsigned count)
{
    DMA1_CH7Send(&SrcFilterHSV, hsv, count);
}

//------------------------------------------------------------
void ws2812b_PA2_SendRGB(RGB_t *rgb, unsigned count)
{
    DMA1_CH1Send(&SrcFilterRGB, rgb, count);
}

void ws2812b_PA2_SendHSV(HSV_t *hsv, unsigned count)
{
    DMA1_CH1Send(&SrcFilterHSV, hsv, count);
}

//------------------------------------------------------------
void ws2812b_PA6_SendRGB(RGB_t *rgb, unsigned count)
{
    DMA1_CH6Send(&SrcFilterRGB, rgb, count);
}

void ws2812b_PA6_SendHSV(HSV_t *hsv, unsigned count)
{
    DMA1_CH6Send(&SrcFilterHSV, hsv, count);
}

//------------------------------------------------------------
void ws2812b_PB1_SendRGB(RGB_t *rgb, unsigned count)
{
    DMA1_CH3Send(&SrcFilterRGB, rgb, count);
}

void ws2812b_PB1_SendHSV(HSV_t *hsv, unsigned count)
{
    DMA1_CH3Send(&SrcFilterHSV, hsv, count);
}

//------------------------------------------------------------
void ws2812b_PA8_SendRGB(RGB_t *rgb, unsigned count)
{
    DMA1_CH2Send(&SrcFilterRGB, rgb, count);
}

void ws2812b_PA8_SendHSV(HSV_t *hsv, unsigned count)
{
    DMA1_CH2Send(&SrcFilterHSV, hsv, count);
}
