#ifndef __CMT2300_HAL_H
#define __CMT2300_HAL_H

//#include "typedefs.h"
#include "spi.h"

#ifdef __cplusplus 
extern "C" { 
#endif

/* ************************************************************************
*  The following need to be modified by user
*  ************************************************************************ */
#define CMT2300A_SetGpio1In()           SET_GPIO_IN_NOINT(CMT_GPIO1_GPIO)
#define CMT2300A_SetGpio2In()           SET_GPIO_IN_NOINT(CMT_GPIO2_GPIO)
#define CMT2300A_SetGpio3In()           SET_GPIO_IN_NOINT(CMT_GPIO3_GPIO)
#define CMT2300A_SetRF433Out()          SET_GPIO_OUT(CMT_GPIO1_GPIO)
	
#define CMT2300A_ReadGpio1()            READ_GPIO_PIN(CMT_GPIO1_GPIO)
#define CMT2300A_ReadGpio2()            READ_GPIO_PIN(CMT_GPIO2_GPIO)
#define CMT2300A_ReadGpio3()            READ_GPIO_PIN(CMT_GPIO3_GPIO)
#define CMT2300A_DelayMs(ms)            system_delay_ms(ms)
#define CMT2300A_DelayUs(us)            system_delay_us(us)
#define CMT2300A_GetTickCount()         g_nSysTickCount
/* ************************************************************************ */


#ifdef __cplusplus 
} 
#endif
void Cmt2300_InitGpio(void);
void CMT2300A_InitGpio(void);//for FSK
u8 Cmt2300_ReadReg(u8 addr);
void Cmt2300_WriteReg(u8 addr, u8 dat);

void Cmt2300_ReadFifo(u8 buf[], u16 len);
void Cmt2300_WriteFifo(const u8 buf[], u16 len);//

#endif
