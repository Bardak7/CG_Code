/*************** (C) COPYRIGHT  EW工作室 ***************************************
 * 文件名  ：uart.h
 * 描述    ：串口调试文件   
 * 实验平台：EW STM8入门板 V1.0
 * 库版本  ：V2.1.0
 * QQ群	   ：261825684
 * 修改时间：2013-08-20
*******************************************************************************/
#ifndef __UART_H
#define __UART_H

#include "stm8s.h"

void Uart1_Init(void);
void UART1_Send(u8 dat);
void UART1_SendByte(u8 data);
void UART1_SendString(u8* Data);

void putchar(char ch);
void printf_str(unsigned char *buffer);
void putascbase(unsigned char ch);
void putasc(unsigned char ch);
void putascLong(unsigned long ch);
void putascShort(unsigned short ch);
void printf_array(unsigned char *buffer, unsigned short  cnt) ;


#endif

/******************* (C) COPYRIGHT EW工作室 *****END OF FILE******************/