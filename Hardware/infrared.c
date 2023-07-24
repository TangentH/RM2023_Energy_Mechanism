#include "stm32f10x.h" // Device header
#include "infrared.h"
#include "timer.h"
#include "led.h"
#include "ws2812b.h"

uint8_t timerOFFSET = 1; // 当系统频率为8Mhz时需要做补偿，可以通过主函数中的RCC_Clocks结构体获得当前时钟频率

uint16_t IR_Time; // 存储每两个下降沿之间的时间
uint8_t IR_State; // 状态，空闲 = 0，接收起始/重复段 = 1，解码 = 2

uint8_t IR_Data[4]; // 存储接收到的4段数据，每段数据8位
uint8_t IR_pData;   // 用于指示正在接收哪一位数据，最大为32

uint8_t IR_DataFlag;    // 接收结束标志，接收结束则置1
uint8_t IR_RepeatFlag;  // 重复标志，若接收到重复帧则置1
uint8_t IR_Address;     // 用于存储地址码
uint8_t IR_Command;     // 用于存储命令码（键码）
uint8_t IR_AntiAddress; // 用于存储地址码反码
uint8_t IR_AntiCommand; // 用于存储命令码反码（键码）

extern RGB_t current_color; //变量，表示这片叶子亮哪方的颜色（其实有后面的LED_State就可以判断了，但是为了减轻CPU负担，尽量在颜色判断上只在初始化的时候判断一次）
extern LED_State_t LED_State; //能量机关默认为红方模式
extern uint8_t currentLeafStruck;
extern LED_Leaf_Name_t current_striking_leaf; //当前击打的扇叶
extern uint8_t leaf_ring_value[5]; //用于存储每片扇叶的环数
extern RGB_t current_color;
extern uint8_t total_struck;    //总共击打的次数
/*
        NEC红外编码: 引导码 + 地址码 + 地址码(取反) + 数据 + 数据(取反)

        引导码：持续9ms 高电平，4.5ms低电平，作为启动信号；
        紧接着是32bit的数据，按照上述的NEC帧格式的顺序；
        最后以562.5μs脉冲高电平结尾，表示一帧消息传输结束。
        逻辑“0”：562.5μs高电平，562.5μs低电平，总时长为1.125ms
        逻辑“1”：562.5μs高电平，1.6875ms低电平，总时长为2.25ms
        重复码：9ms 前导高电平，2.25ms的低电平，562.5μs的高电平来标记一帧重复码的结束。
*/
void EXTI3_Init(void);
void LED_set(void);

void IR_Init(void)
{
    EXTI3_Init();
}

uint8_t IR_GetDataFlag(void)
{
    if (IR_DataFlag)
    {
        IR_DataFlag = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t IR_GetRepeatFlag(void)
{
    if (IR_RepeatFlag)
    {
        IR_RepeatFlag = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t IR_GetAddress(void)
{
    return IR_Address;
}

uint8_t IR_GetCommand(void)
{
    return IR_Command;
}

uint8_t IR_GetAntiAddress(void)
{
    return IR_AntiAddress;
}

uint8_t IR_GetAntiCommand(void)
{
    return IR_AntiCommand;
}

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0)
        {
            if (IR_State == 0)
            {
                TIM4_StartCounter();
                IR_State = 1;
            }
            else if (IR_State == 1)
            {
                IR_Time = TIM4_GetCounter();
                TIM4_StartCounter();
                if (IR_Time > 13500 - 500 && IR_Time < 13500 + 500)
                {
                    IR_State = 2;
                }
                else if (IR_Time > 11250 - 500 && IR_Time < 11250 + 500)
                {
                    IR_RepeatFlag = 1;
                    IR_State = 0;
                }
                else
                {
                    IR_State = 1;
                }
            }
            else if (IR_State == 2) // 接收数据
            {
                IR_Time = TIM4_GetCounter();
                TIM4_StartCounter();

                if (IR_Time > 1120 - 500 && IR_Time < 1120 + 300) // 逻辑0
                {
                    IR_Data[IR_pData / 8] &= ~(0x01 << (IR_pData % 8));
                    IR_pData++;
                }
                else if (IR_Time > 2250 - 500 && IR_Time < 2250 + 500) // 逻辑1
                {
                    IR_Data[IR_pData / 8] |= (0x01 << (IR_pData % 8));
                    IR_pData++;
                }
                else // 既不是逻辑0也不是逻辑1 接收出错的情况
                {
                    IR_pData = 0; // 重置IR_pData
                    IR_State = 1; // 不接收此帧数据，寻找下一帧起始码
                }
                if (IR_pData >= 32)
                {

                    IR_pData = 0;
                    //					if((IR_Data[0] == ~IR_Data[1]) && (IR_Data[2] == ~IR_Data[3]))
                    //					{	//这里检查一直不通过，导出反码之后发现反码都是零
                    IR_Address = IR_Data[0];
                    IR_AntiAddress = IR_Data[1];
                    IR_Command = IR_Data[2];
                    IR_AntiCommand = IR_Data[3];
                    IR_DataFlag = 1;    // 接收结束

                    //					}
                    IR_State = 0;
									if(IR_Address == 0x00 )     //防止出现全部点亮后，还能继续击打的情况
                    {
                        LED_set();  //跟据接收到的红外信号，改变能量机关的状态
                    }
                }
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI3_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿激发中断
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
		//由于这份代码太多中断了，为了防止错过红外信号，要提高它的NVIC优先级
}

void LED_set(void)
{
    switch(IR_Command)
    {
        case 0x15:  //确定键
            currentLeafStruck = 1;
            leaf_ring_value[current_striking_leaf] = 0;
            break;
        case 0x16:  //上键
            currentLeafStruck = 1;
            leaf_ring_value[current_striking_leaf] = 1;
            break;
        case 0x18:  //右键
            currentLeafStruck = 1;
            leaf_ring_value[current_striking_leaf] = 2;
            break;
        case 0x17:  //下键
            currentLeafStruck = 1;
            leaf_ring_value[current_striking_leaf] = 3;
            break;
        case 0x19:  //左键
            currentLeafStruck = 1;
            leaf_ring_value[current_striking_leaf] = 4;
            break;
        case 0x94:
            if(LED_State == RedState)
            {
                LED_State = BlueState;
                current_color = blue;
            }
            else
            {
                LED_State = RedState;
                current_color = red;
            }
            break;
    }
}
