#include "tim2.h"

/**

 ��ʱ��2�ĳ�ʼ��

**/
void time2_Init(void)
{
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_1,5000);  //ʱ���ĳ�ʼ��
}