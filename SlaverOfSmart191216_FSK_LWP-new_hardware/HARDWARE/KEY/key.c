#include "sys.h" 
#include "cmt2300.h"
#include "IRorRF.h"
#include "stmflash.h"
#include "cmt2300.h"
#include "radio.h"
extern SysStruct SysSetData;
void KEY_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	  //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
extern IrOrRf_DATA rf433_txdata;
extern u8 ASDataCnt,ASTimes,Rf433SendDlyCnt;
//按键使用定时器20ms置旗标扫描一次
void Task_key(void)
{	
	static u8 KeyCnt;
	#ifdef EE_ENABLE
	u16 Tmp[2];
	#endif
  if(Key == 0)
	{	 
	  KeyCnt++;
		SysSetData.tim.sec=0;
		if(KeyCnt>99) 
		{
			KeyCnt = 0;
			//擦除主机地址
			//if(SysSetData.bWasAdd)
			{

				#ifdef EE_ENABLE
				Tmp[0] = 0xffff;
				Tmp[1] = 0xffff;
				INTX_DISABLE();
				STMFLASH_Write(FLASH_SAVE_ADDR1,Tmp,2);
				INTX_ENABLE();
				#endif
				SysSetData.MasterAddr = 0;//未分配主机
				
				ACT_LED_EN;
				Cmt2300_ExitDutyCycle(); //退出全自动模式并进入添加状态。
        SysSetData.key_press=1;
				Cmt2300_GoRx(); //进入接收状态
        while(Key==0){
				 	MCU_LED_EN;
					delay_ms(100);
					MCU_LED_DIS;
					delay_ms(100);
				}
			}		
		}else if(KeyCnt>5){
		  SysSetData.key_press=0;
		}
	}
	else
	{
		KeyCnt = 0;
	}
}
