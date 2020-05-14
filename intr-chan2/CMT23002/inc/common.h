#ifndef __COMMON_H
#define __COMMON_H

//#include "typedefs.h"
#include "stm8s_gpio.h"

//#include "time_server.h"

void no_optimize(const void* p_param);
void Common_Init(void);

void GPIO_Config(void);
void GPIO_Pin_Setting(GPIO_TypeDef *gpio, GPIO_Pin_TypeDef nPin,GPIO_Mode_TypeDef mode)

void set_u16_to_buf(u8 buf[], u16 dat16);
u16 get_u16_from_buf(const u8 buf[]);

void set_u32_to_buf(u8 buf[], u32 dat32);
u32 get_u32_from_buf(const u8 buf[]);

void views_print_line(u8 nLine, const char* str);

#endif
