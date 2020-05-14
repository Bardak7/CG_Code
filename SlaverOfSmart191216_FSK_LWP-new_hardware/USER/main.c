//===================================================
//	 项目名：从机设备
//	 日期：2016.10.21
//	 MCU：STM32F103RB
//	 功能按键长按,WiFi重配   短按,主从机配对(通过无线)
//   备注：
//        1.上电，闪2次.正常工作状态时，短亮5s。欠电时,短亮10s
//  1.从机作为中继设备，不设锁定。锁定功能由主机完成
//  2.非库红外码传至服务器
//  3.从机定时任务，在主机处备份，以便app查询定时列表时用。从机的定时任务由从机自主完成
//  4.主从机之间通过433通讯
//
//  16.10.31 添加分机，一经掉电，不记忆。因为没有删除添加命令
//  红外用于学习及发送码库数据
//  433M用于传递主分机之间，分机与开关之间的通讯
//  定时任务暂时针对于无线开关
//  主机在向从机发送添加成功数据时，携带时间信息
//===================================================
//  17.05.05
//  1.从机不存储定时任务信息，有主机存储并发送
//  2.由于从机是电池供电，因此增加电压检测。电压不足时led亮1灭3.正常亮1灭1.
//  3.在添加主机后，存储主机地址及本机状态
//  4.长按按键，擦除已配对主机id
//  5.test_led1可作为长按取消主机地址闪三次，有数据通讯闪
//  17.11.13
//  6.掉电时向主机发送状态信息
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
	// HCLK = SYSCLK/4 =12M    这里是AHB的分频不知道为什么写这里了2018-5-12 author: connor 
	RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1; //AHB不分频
	RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1; //APB1 不分频
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
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	  //电源控制部分的时钟打开 
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);	
//	PWR_ClearFlag (PWR_FLAG_WU);
	PWR_WakeUpPinCmd(ENABLE);//设置PA0为WKUP引脚上升沿唤醒
	
	PWR_EnterSTANDBYMode();
}
extern IrOrRf_DATA rf433_txdata;

int main(void)
{
  
	RCC_Configuration();
	delay_init(16);	     //延时初始化
	NVIC_Configuration();//中断设置
	LED_Init();	
	KEY_Init();          //按键初始化	，暂时未定其功能
  IRorRF_Init();	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); //使能低功耗相应的时钟
	PWR->CR &= ~PWR_CR_CWUF;  //唤醒先不清除唤醒标志
//	PWR_ClearFlag (PWR_FLAG_WU);
	DataInit();
	RF_Init_FSK(TRUE);
	Timer2_Init(10-1,1600-1);	     //1ms 到达定时器的时钟是：8/4 *2 /psc
	Timer3_Init(14-1,16-1);        //14us 红外振荡  把14改成 13  20200415
	Timer1_Init(50-1,16-1);				 //50us 脉宽时基

	  MCU_LED_EN;

    EXTIX_Init();        //433,ir载波由定时中断产生
  DBGMCU_Config(DBGMCU_STANDBY,ENABLE);

	while(1)
	{  

	  OnSlave();
		TaskTime();           //时基任务
		Task_rf433();
		if(Task20msFlag)	    //20ms级任务
		{
			Task20msFlag=0;
			Task_key();		      //按键扫描-擦除主机地址
		}	 
			
		if(Task1minFlag == 1)
		{
			MCU_LED_DIS;
			Task1minFlag=0;
			temp=1;
			if(SysSetData.key_press==0){
				if(CMT2300A_CheckLowpowerMode()==FALSE){//如果没有进入低功耗才进入低功耗
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

