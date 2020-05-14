
/*************** (C) COPYRIGHT  EW������ ***************************************
 * �ļ���  ��sysclock.c
 * ����    ��ϵͳʱ������    
 * ʵ��ƽ̨��EW STM8���Ű� V1.0
 * ��汾  ��V2.1.0
 * QQȺ	   ��261825684
 * �޸�ʱ�䣺2013-08-20
*******************************************************************************/

/* ����ϵͳͷ�ļ� */

/* �����Զ���ͷ�ļ� */
#include "sysclock.h"
#include "stm8s_clk.h"
#include "stm8s_it.h"
/* �Զ��������� */

/* �Զ���� */

//#define HSE_Enable

/*******************************************************************************
 * ����: Sysclock_Init
 * ����: ����ϵͳʱ��Ƶ��
 * �β�: ��
 * ����: ��
 * ˵��: ʱ��ѡ��

 ******************************************************************************/
void SystemClock_Init(void)
{
//ʹ�ú궨���д��
#ifdef  HSE_Enable
  CLK_HSECmd(ENABLE);   

  while (!CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE,\
            CLK_CURRENTCLOCKSTATE_DISABLE));
     //�л����ⲿʱ��,���ȴ�ʱ��ȴ���ɹ�
    CLK_ClockSecuritySystemEnable();
    //CLK_ITConfig(CLK_IT_CSSD,ENABLE);  //ϵͳʱ�Ӱ�ȫ�ж�
    //CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);
#else
    //CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);
     CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
#endif   

}


/******************* (C) COPYRIGHT EW������ *****END OF FILE******************/

