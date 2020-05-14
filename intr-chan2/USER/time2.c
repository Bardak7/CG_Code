#include "tim2.h"

/**

 定时器2的初始化

**/
void time2_Init(void)
{
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_1,5000);  //时基的初始化
}