
#include "tim1.h"
#include "uart.h"
#include "stm8s_itc.h"


/*******************************************************************************
 * ����: void TIM1_Init(void)
 * ����: TIM1��ʼ��
 * �β�: ��
 * ����: ��
 * ˵��: ��
 ******************************************************************************/
void TIM1_Init(void)
{
	TIM1_DeInit();
	/* ��ʼ��TIM1ΪHSI 16��Ƶ ���ϼ���ģʽ ��������20 ������ʼֵ0 -- 20us��һ���ж�*/
	TIM1_TimeBaseInit(16, TIM1_COUNTERMODE_UP, 20, 0);
	TIM1_SetCounter(0);						/* ����������ֵ��Ϊ0 */
	TIM1_ARRPreloadConfig(ENABLE);			/* Ԥװ��ʹ�� */
	
	
	TIM1_ITConfig(TIM1_IT_UPDATE , ENABLE);	/* ���������ϼ���/���¼�����������ж� */
        ITC_SetSoftwarePriority(ITC_IRQ_TIM1_OVF, ITC_PRIORITYLEVEL_3);  //������ȼ���ߡ�
	TIM1_Cmd(ENABLE);						
}
/* ��ʼ��TIM2ΪHSI 32768 ��Ƶ ���ϼ���ģʽ ��������200 ������ʼֵ0 -- 400MS���ҽ�һ���ж�
  16000000/32768 =488.28125     Ƶ�ʵĵ���ʱ������2MS����һ��
*/
 void TIM2_Init(void)
{
   TIM2_DeInit();
   TIM2_TimeBaseInit(TIM2_PRESCALER_32768,200);
   TIM2_SetAutoreload(200);
   TIM2_SetCounter(0);
   TIM2_ITConfig(TIM2_IT_UPDATE,ENABLE);
   ITC_SetSoftwarePriority(ITC_IRQ_TIM2_OVF, ITC_PRIORITYLEVEL_1);
   TIM2_Cmd(ENABLE);
}