
#include "tim1.h"
#include "uart.h"
#include "stm8s_itc.h"


/*******************************************************************************
 * 名称: void TIM1_Init(void)
 * 功能: TIM1初始化
 * 形参: 无
 * 返回: 无
 * 说明: 无
 ******************************************************************************/
void TIM1_Init(void)
{
	TIM1_DeInit();
	/* 初始化TIM1为HSI 16分频 向上计数模式 计数周期20 计数初始值0 -- 20us进一次中断*/
	TIM1_TimeBaseInit(16, TIM1_COUNTERMODE_UP, 20, 0);
	TIM1_SetCounter(0);						/* 将计数器初值设为0 */
	TIM1_ARRPreloadConfig(ENABLE);			/* 预装载使能 */
	
	
	TIM1_ITConfig(TIM1_IT_UPDATE , ENABLE);	/* 计数器向上计数/向下计数溢出更新中断 */
        ITC_SetSoftwarePriority(ITC_IRQ_TIM1_OVF, ITC_PRIORITYLEVEL_3);  //软件优先级最高。
	TIM1_Cmd(ENABLE);						
}
/* 初始化TIM2为HSI 32768 分频 向上计数模式 计数周期200 计数初始值0 -- 400MS左右进一次中断
  16000000/32768 =488.28125     频率的倒数时间差不多是2MS计数一次
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