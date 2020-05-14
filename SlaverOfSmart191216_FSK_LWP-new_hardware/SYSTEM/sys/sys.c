#include "sys.h"
#include "stmflash.h"
#include "cmt2300.h"
#include "IRorRF.h"
u8 Task20msFlag;
u8 Task1minFlag;
u8 ASDataFlag;  //补发旗标
u8 ASTimes;     //补发次数
u8 ASDataCnt;
extern u8 Rec433OkFlag;//收到旗标
extern u8 Rec433OkCnt;
extern IrOrRf_DATA rf433_txdata;
extern u8 Task1msCnt;
extern SysStruct SysSetData;
extern u16 HeadSpaceCnt;
extern u16 SpaceCnt;
extern u16 TailHCnt;
extern u16 TailLCnt;
//======================================================
void NVIC_Configuration(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
}


void AddSendData(void)//补发函数 补发次数，补发间隔20ms单位
{
	if(ASDataFlag)
	{

		ASDataCnt--; //每200MS加一次
		if(ASDataCnt==0)//1s
		{
			ASDataFlag=0;
			SysSetData.Rf433Send_flag=1;
	  }
	}
}

void TaskTime(void)
{
	static u8 Task20msCnt,Task200msCnt;//LedFlag,
	
	if(Task1msCnt>19)
	{
		Task1msCnt=0;
		Task20msFlag=1;
		Task20msCnt++;

		if(Task20msCnt>9)
		{
			Task20msCnt=0;
			Task200msCnt++;
			
			if(Task200msCnt>4) //1s
			{	
				ACT_LED_DIS;
				Task200msCnt=0;
				SysSetData.tim.sec++;		

				if(SysSetData.tim.sec>30)//sec,59
				{	
					Task1minFlag=1;
					SysSetData.tim.sec=0;
					SysSetData.tim.min++;

				}
			}
		}

		if(Rec433OkFlag)
		{
			Rec433OkCnt++;//20ms
			if(Rec433OkCnt>AddSendTime)
			{
				Rec433OkCnt=0;
				Rec433OkFlag=0;
			}
		}
		
		AddSendData();//1s重发
	}
}

void DataInit(void)
{
	u16 Tmp[2];
	SysSetData.SysAddr=*(vu32*)(FLASH_SAVE_ADDR0);//读取本机地址
	STMFLASH_Read(FLASH_SAVE_ADDR1,Tmp,2);//读取主机地址
	
	if((Tmp[0]==0xffff)&&(Tmp[1]==0xffff))
	{
		SysSetData.MasterAddr = 0;//未分配主机
		SysSetData.bWasAdd = 0;    
	}
	else if((Tmp[0]!=0xffff)||(Tmp[1]!=0xffff))
	{
		SysSetData.MasterAddr = Tmp[1];
		SysSetData.MasterAddr <<= 16;
		SysSetData.MasterAddr += Tmp[0];
		SysSetData.bWasAdd = 1;    //已分配主机标志
	}
	Rf433TxSocket();
//	Task1minFlag = 1;
}

//关闭所有中断
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}

