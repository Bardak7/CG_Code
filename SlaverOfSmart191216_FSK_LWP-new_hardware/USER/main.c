//===================================================
//	 ��Ŀ�����ӻ��豸
//	 ���ڣ�2016.10.21
//	 MCU��STM32F103RB
//	 ���ܰ����c����,WiFi����   �̰�,���ӻ����(ͨ������)
//   ��ע��
//        1.�ϵ磬��2��.��������״̬ʱ������5s��Ƿ��ʱ,����10s
//  1.�ӻ���Ϊ�м��豸�����������������������������
//  2.�ǿ�����봫��������
//  3.�ӻ���ʱ���������������ݣ��Ա�app��ѯ��ʱ�б�ʱ�á��ӻ��Ķ�ʱ�����ɴӻ��������
//  4.���ӻ�֮��ͨ��433ͨѶ
//
//  16.10.31 ��ӷֻ���һ�����磬�����䡣��Ϊû��ɾ���������
//  ��������ѧϰ�������������
//  433M���ڴ������ֻ�֮�䣬�ֻ��뿪��֮���ͨѶ
//  ��ʱ������ʱ��������߿���
//  ��������ӻ�������ӳɹ�����ʱ��Я��ʱ����Ϣ
//===================================================
//  17.05.05
//  1.�ӻ����洢��ʱ������Ϣ���������洢������
//  2.���ڴӻ��ǵ�ع��磬������ӵ�ѹ��⡣��ѹ����ʱled��1��3.������1��1.
//  3.����������󣬴洢������ַ������״̬
//  4.�����������������������id
//  5.test_led1����Ϊ����ȡ��������ַ�����Σ�������ͨѶ��
//  17.11.13
//  6.����ʱ����������״̬��Ϣ
//===================================================
#include "sys.h"
#include "cmt2300.h"
#include "IRorRF.h"
#include "spi.h"
#include "radio.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "timer.h"
#include "cmt2300_defs.h"
#include "cmt2300_hal.h"
extern u8 Task20msFlag;
extern u8 Task1minFlag;
extern SysStruct SysSetData;
u8  temp=0x00;
void RCC_Configuration(void)
{
	// Set HSION bit 
	RCC->CR |= (uint32_t)0x00000001;//1: internal 8 MHz RC oscillator ON
	// select HSI as PLL source
	RCC->CFGR |= (uint32_t)RCC_CFGR_PLLSRC_HSI_Div2;
	// PLLCLK=8/2*12=48M
	RCC->CFGR |= (uint32_t)RCC_CFGR_PLLMULL4;  //8/2*2=12RCC_CFGR_PLLMULL4
	// HCLK = SYSCLK/4 =12M    ������AHB�ķ�Ƶ��֪��Ϊʲôд������2018-5-12 author: connor 
	RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1; //AHB����Ƶ
	RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1; //APB1 ����Ƶ
	// Enable PLL 
	RCC->CR |= RCC_CR_PLLON;    //1: PLL ON
	// Wait till PLL is ready 
	while((RCC->CR & RCC_CR_PLLRDY) == 0)
	{}

	// Select PLL as system clock source 
//	RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW)); // 00: HSI selected as system clock
	RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    //10: PLL selected as system clock
					
	// Wait till PLL is used as system clock source 
	while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
	{}
}
void Sys_StandbyEntry(void)
{
//	RF_Init_FSK(TRUE);
//	GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	  //��Դ���Ʋ��ֵ�ʱ�Ӵ� 
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);	
//	PWR_ClearFlag (PWR_FLAG_WU);
	PWR_WakeUpPinCmd(ENABLE);//����PA0ΪWKUP���������ػ���
	
	PWR_EnterSTANDBYMode();
}
extern IrOrRf_DATA rf433_txdata;

int main(void)
{
  
	RCC_Configuration();
	delay_init(16);	     //��ʱ��ʼ��
	NVIC_Configuration();//�ж�����
	LED_Init();	
	KEY_Init();          //������ʼ��	����ʱδ���书��
  IRorRF_Init();	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); //ʹ�ܵ͹�����Ӧ��ʱ��
	PWR->CR &= ~PWR_CR_CWUF;  //�����Ȳ�������ѱ�־
//	PWR_ClearFlag (PWR_FLAG_WU);
	DataInit();
	RF_Init_FSK(TRUE);
	Timer2_Init(10-1,1600-1);	     //1ms ���ﶨʱ����ʱ���ǣ�8/4 *2 /psc
	Timer3_Init(14-1,16-1);        //14us ������  ��14�ĳ� 13  20200415
	Timer1_Init(50-1,16-1);				 //50us ����ʱ��

	  MCU_LED_EN;

    EXTIX_Init();        //433,ir�ز��ɶ�ʱ�жϲ���
  DBGMCU_Config(DBGMCU_STANDBY,ENABLE);

	while(1)
	{  

	  OnSlave();
		TaskTime();           //ʱ������
		Task_rf433();
		if(Task20msFlag)	    //20ms������
		{
			Task20msFlag=0;
			Task_key();		      //����ɨ��-����������ַ
		}	 
			
		if(Task1minFlag == 1)
		{
			MCU_LED_DIS;
			Task1minFlag=0;
			temp=1;
			if(SysSetData.key_press==0){
				if(CMT2300A_CheckLowpowerMode()==FALSE){//���û�н���͹��ĲŽ���͹���
					CMT2300A_LowpowerMode(TRUE);
				}
				temp=0x00;
				RCC_APB2PeriphClockCmd(0x01fc,DISABLE);
				Sys_StandbyEntry();

			}
			Task1minFlag=0;
		}
	}	 
}

