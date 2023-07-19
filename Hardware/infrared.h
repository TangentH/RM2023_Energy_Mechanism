#ifndef __INFRARED_H__
#define __INFRARED_H__

void IR_Init(void);
uint8_t IR_GetDataFlag(void);
uint8_t IR_GetRepeatFlag(void);
uint8_t IR_GetAddress(void);
uint8_t IR_GetCommand(void);

#endif
