#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long
void HlwIo_Init(void);
void U1_Init(u32 bound);
void Send_wifi(void);
void WifiTxSocket(void);
void Task_wifi(void);

#endif
