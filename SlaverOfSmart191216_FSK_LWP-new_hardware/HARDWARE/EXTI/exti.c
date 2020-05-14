#include "sys.h"

//Pc11外部中断
void EXTIX_Init(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	//PB6  -Rf433In
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource6);
	EXTI_InitStructure.EXTI_Line=EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								
	NVIC_Init(&NVIC_InitStructure);  	  
	
}

extern SysStruct SysSetData;
u16 HeadSpaceCnt;
u16 SpaceCnt;
u16 TailHCnt;
u16 TailLCnt;
enum rev_func rf433_rcvstat;
struct IsIrInt rf433_intstat;
u8 rf433_data[300],Rf433ByteIndex;
void EXTI9_5_IRQHandler(void)//433
{
	static u8 tRf433Data,Rf433BitIndex;//临时变量，位序列，字节序列
	
	if(SysSetData.Rf433Send_flag == 0)//未在发射状态
	{
		if(Rf433In == 0)//下降沿
		{
			if(rf433_rcvstat == nowork)//未收到头
			{
				//if((rf433_intstat.cnt > 372) && (rf433_intstat.cnt < 412))   //5500/14us=392
				if((rf433_intstat.cnt > 340) && (rf433_intstat.cnt < 440))
				{
					rf433_rcvstat = inwork;  //head ok
					Rf433BitIndex = 0;  //bit下标
					Rf433ByteIndex = 0; //字节下标
					tRf433Data = 0;
				}
			}
			else if(rf433_rcvstat == inwork)
			{ 
				if((rf433_intstat.cnt > 32) && (rf433_intstat.cnt < 52))   //"1",42.8
				{
					tRf433Data |= (1 << Rf433BitIndex);
					Rf433BitIndex++;
				}
				else if((rf433_intstat.cnt > 4) && (rf433_intstat.cnt < 24))  //"0",14.2
				{
					Rf433BitIndex++;
				}
				else
				{
					rf433_rcvstat = nowork;
				}
				
				if(Rf433BitIndex == 8)
				{
					Rf433BitIndex = 0;
					rf433_data[Rf433ByteIndex++] = tRf433Data;
					tRf433Data = 0;
					
					if(Rf433ByteIndex>10)
						if((12+rf433_data[11]) == Rf433ByteIndex)
						{
							rf433_rcvstat = workok;
						}
				}
			}
		
			rf433_intstat.cnt = 0;
			rf433_intstat.flag = 0;
		}
		else            //上升沿
		{
			rf433_intstat.cnt = 0;
			rf433_intstat.flag = 1;//开始计时
		}
  }
	EXTI_ClearITPendingBit(EXTI_Line6);  
}
 
