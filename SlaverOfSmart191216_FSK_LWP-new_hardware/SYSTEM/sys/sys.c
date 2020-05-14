#include "sys.h"
#include "stmflash.h"
#include "cmt2300.h"
#include "IRorRF.h"
u8 Task20msFlag;
u8 Task1minFlag;
u8 ASDataFlag;  //�������
u8 ASTimes;     //��������
u8 ASDataCnt;
extern u8 Rec433OkFlag;//�յ����
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
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
}


void AddSendData(void)//�������� �����������������20ms��λ
{
	if(ASDataFlag)
	{

		ASDataCnt--; //ÿ200MS��һ��
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
		
		AddSendData();//1s�ط�
	}
}

void DataInit(void)
{
	u16 Tmp[2];
	SysSetData.SysAddr=*(vu32*)(FLASH_SAVE_ADDR0);//��ȡ������ַ
	STMFLASH_Read(FLASH_SAVE_ADDR1,Tmp,2);//��ȡ������ַ
	
	if((Tmp[0]==0xffff)&&(Tmp[1]==0xffff))
	{
		SysSetData.MasterAddr = 0;//δ��������
		SysSetData.bWasAdd = 0;    
	}
	else if((Tmp[0]!=0xffff)||(Tmp[1]!=0xffff))
	{
		SysSetData.MasterAddr = Tmp[1];
		SysSetData.MasterAddr <<= 16;
		SysSetData.MasterAddr += Tmp[0];
		SysSetData.bWasAdd = 1;    //�ѷ���������־
	}
	Rf433TxSocket();
//	Task1minFlag = 1;
}

//�ر������ж�
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//���������ж�
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}

