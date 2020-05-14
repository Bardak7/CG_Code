#ifndef __ADC_H
#define __ADC_H	

#include "typedefs.h"

void T_AdcTemp_Init(void);//ADC通道初始
u16 T_Get_AdcTemp(u8 ch);  
u16 T_Get_Temp(void);
u16 T_Get_Average(u8 ch,u8 times);
short Get_Temprate(void);	

void T_AdcVolt_Init(void);//ADC通道初始
u16 T_Get_AdcVolt(u8 ch);  
u16 T_Get_Volt(void);
u16 T_Get_Average1(u8 ch,u8 times);
short Get_Volt(void);	

#endif
