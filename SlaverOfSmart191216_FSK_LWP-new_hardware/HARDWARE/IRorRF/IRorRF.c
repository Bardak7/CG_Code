#include "sys.h"
#include "cmt2300.h"
#include "IRorRF.h"
#include "stmflash.h"
#include "radio.h"
u8 pwda[] = {0x94,0x9D,0x3C,0xAA,0x35,0x10,0x5B,0x5E,0x55,0x71,0x51,0x86,0x25,0x8A,0xD8,0xAC,0x98,0xB1,0xFE,0x05,\
             0xF2,0x82,0x2E,0x1C,0x7E,0x9F,0x40,0xDC,0x46,0xF7,0x7C,0x3D,0x7D,0x4D,0x45,0xD6,0xF5,0x49,0xD9,0x76,\
             0xE3,0xED,0x70,0xE6,0x00,0x23,0xB2,0x6C,0xA4,0xEF,0xA5,0x31,0xC4,0xC0,0xEC,0x5F,0x54,0xD1,0xCC,0xCD,\
             0xB7,0x64,0x12,0x0F,0x04,0xDB,0x27,0xBA,0xF6,0x1B,0xFD,0xB4,0x38,0xD0,0x02,0xDD,0xA0,0x11,0x18,0xC2,\
             0xC5,0x07,0x4F,0xF9,0xC6,0xB6,0xFA,0x2B,0xCB,0xCA,0x13,0x0C,0x2A,0xB5,0x3F,0x0A,0xD5,0x15,0x89,0x7A,\
             0x72,0x8D,0x3E,0x43,0x96,0x06,0x22,0x9B,0xF8,0x42,0xF3,0x93,0x9A,0x24,0x6E,0x0E,0x26,0xAD,0xB3,0xBB,\
             0xB8,0x87,0x41,0xE2,0xC7,0xA1,0x0B,0x16,0x61,0x5C,0x4B,0x5D,0xA8,0x57,0x66,0xE4,0xFF,0xC8,0xC1,0xBF,\
             0x29,0xD2,0xDF,0x60,0xEE,0x37,0x6F,0xAB,0xE9,0x48,0x58,0xDE,0x1D,0x8C,0x1E,0x17,0x1F,0xE1,0x9C,0x14,\
             0x56,0x8F,0x34,0xC9,0x3B,0x28,0x7F,0x99,0xD7,0xAE,0x59,0xBD,0xAF,0x65,0xF1,0x81,0xEA,0x19,0x01,0x74,\
             0x90,0x08,0x03,0x4C,0x20,0x79,0x62,0x39,0xFC,0x1A,0xC3,0x73,0x53,0xBE,0x09,0x69,0xA6,0xD3,0xE8,0xCF,\
             0x63,0x3A,0x77,0x47,0x32,0x4A,0x7B,0x5A,0xE0,0xE5,0xA2,0xA7,0x50,0x83,0x6B,0x44,0xE7,0xA3,0x21,0xFB,\
             0xF4,0x9E,0x30,0xF0,0x2C,0x33,0x2F,0x67,0x36,0xCE,0x80,0x75,0x6D,0x8B,0x85,0xBC,0xB9,0x8E,0x92,0x95,\
             0xA9,0x2D,0x4E,0x68,0x52,0xD4,0xEB,0x6A,0xB0,0x84,0x78,0xDA,0x97,0x88,0x0D,0x91};

u8 pwdb[] = {0x27,0x33,0x8A,0x91,0x13,0xC4,0x46,0x6F,0xAD,0xAC,0xA8,0x4A,0x70,0xEB,0xB8,0xDC,0x8D,0x2E,0xBA,0x49,\
             0x02,0x2D,0x4E,0x79,0xD2,0xD3,0x99,0x8B,0xD4,0x7A,0x3C,0x18,0x20,0xCB,0x98,0x03,0x86,0xFC,0x84,0x72,\
             0x23,0x17,0x2A,0x12,0xBB,0xA1,0x1B,0xB9,0x34,0xE0,0xDF,0xE6,0x26,0xF1,0x66,0x3A,0x6C,0xC7,0xF5,0xF2,\
             0x0E,0x9E,0x5B,0x0D,0x71,0x83,0x21,0x57,0x52,0xCD,0x7C,0xA0,0x37,0x97,0xB6,0xBF,0x08,0xC3,0x53,0x07,\
             0xC0,0xA5,0xA9,0x76,0xA7,0x9C,0xAA,0x64,0x0B,0x94,0x55,0xA3,0xAF,0xCE,0x6D,0x56,0x95,0x1C,0xDA,0xA4,\
             0xE9,0x90,0xFE,0x9D,0x5F,0x4D,0x75,0x87,0x3B,0x2F,0xC8,0xF8,0x32,0xE4,0xA2,0x96,0x19,0x3F,0x10,0x14,\
             0x6B,0xBD,0xC6,0x80,0x0A,0x00,0x16,0x85,0xD0,0xD9,0x44,0xBC,0x81,0x48,0x30,0xC2,0x58,0x24,0x73,0xB5,\
             0x62,0x2C,0xB7,0xC5,0xED,0x61,0x93,0x04,0xF3,0x42,0x78,0x40,0x1E,0xFB,0xA6,0x43,0x3D,0x1F,0x77,0xB0,\
             0xD5,0x7D,0x63,0x7E,0x7B,0x4F,0x47,0x45,0x22,0x5D,0x1D,0x5E,0x6A,0x0C,0x15,0xC9,0xC1,0x92,0x4B,0xE2,\
             0xFF,0xE7,0x74,0x88,0x31,0x8C,0xEA,0x9B,0x0F,0xEF,0xB2,0xD7,0x06,0x4C,0xB1,0xF7,0xCC,0x01,0x65,0xEE,\
             0xD6,0x68,0x50,0x5C,0xCF,0xE1,0xFD,0x05,0x54,0x59,0xE8,0xB4,0x25,0x29,0xCA,0x35,0xE3,0xF9,0x36,0x69,\
             0xBE,0xD8,0xE5,0x5A,0x82,0x28,0x38,0x09,0xF4,0x67,0x60,0x1A,0x8E,0xDD,0x2B,0xAE,0x7F,0x39,0xF0,0x9A,\
             0xFA,0x8F,0x41,0xEC,0x11,0x3E,0xDB,0xF6,0xDE,0xAB,0x6E,0x9F,0xB3,0xD1,0x89,0x51};

u8 pwdc[] = {0x56,0x16,0x91,0x69,0xB8,0x3F,0x8B,0x8A,0x06,0x87,0xC8,0xCF,0xF6,0xB5,0x1A,0xC5,0xFF,0x2F,0x6B,0x3A,\
             0xE5,0x6F,0x2B,0xD3,0xD9,0x3C,0x47,0x8E,0xF8,0x72,0xF0,0x6E,0x94,0xAF,0xA4,0x5F,0x03,0xD8,0xAB,0xCC,\
             0x85,0xF9,0xEC,0xEA,0x54,0x8C,0x07,0xB0,0x75,0x78,0x39,0x64,0x98,0x44,0x34,0x60,0xE8,0x76,0x14,0x52,\
             0x6D,0x4C,0x50,0x05,0x00,0x6A,0xDA,0xDC,0x61,0xF3,0x2E,0x6C,0xFB,0xD4,0x7B,0xAD,0x4E,0xC6,0x0F,0x71,\
             0x42,0xA3,0x77,0x10,0x86,0x9E,0x1F,0x63,0x25,0x31,0xBC,0xDF,0x5A,0xEF,0x7C,0x74,0xB3,0xBE,0x01,0x12,\
             0x7A,0x38,0xFA,0xA1,0x5C,0x0B,0xB9,0xDE,0x55,0x27,0xF2,0x70,0x43,0x9A,0xC4,0x0D,0x8F,0x3E,0x20,0xF7,\
             0x40,0xC7,0x7E,0xC9,0x1E,0x46,0xAC,0xA2,0x32,0x4D,0x41,0x28,0x9B,0xED,0x4A,0x62,0xCE,0x1D,0x11,0x17,\
             0x80,0x58,0x7D,0x08,0x18,0xD1,0xE7,0x5B,0x9F,0x1B,0xF1,0xBA,0x93,0xB4,0x96,0xCD,0x0E,0xFD,0x65,0x48,\
             0x0A,0x45,0xB1,0xE9,0x73,0x82,0xC0,0x33,0xD2,0x53,0x81,0x99,0x22,0x9D,0xAE,0xD7,0x0C,0x97,0xD5,0x9C,\
             0x2D,0xCA,0x79,0x4F,0xDD,0xBB,0xEB,0xE3,0xFC,0x59,0xA5,0x2A,0x83,0xB2,0xEE,0x49,0xA0,0xA8,0x90,0x7F,\
             0xB6,0x4B,0xC3,0xAA,0x36,0x29,0x21,0x30,0x1C,0xB7,0x19,0x51,0xA6,0xCB,0x09,0x66,0x84,0xBD,0xE2,0x88,\
             0x5D,0x26,0x2C,0xE0,0xC2,0x3B,0xD0,0xF5,0x24,0x23,0x68,0x37,0xC1,0xBF,0x95,0x3D,0x04,0x8D,0xA9,0xF4,\
             0x89,0x92,0xFE,0x35,0x5E,0x15,0x57,0xE1,0xE4,0x67,0xDB,0xE6,0x02,0x13,0xA7,0xD6};

u16 IrTxCnt;
u8 Rf433SendDlyCnt;
extern enum rev_func rf433_rcvstat;
extern struct IsIrInt rf433_intstat;
extern u8 rf433_data[300];
IrOrRf_DATA rf433_txdata;
SysStruct SysSetData;
extern u16 SpaceCnt;
extern u16 TailHCnt;
extern u16 TailLCnt;
extern u16 HeadSpaceCnt;
extern u8 ASDataFlag;  //补发旗标
extern u8 ASTimes;     //补发次数
extern u8 ASDataCnt;
u8 Rec433OkFlag;//收到旗标
u8 Rec433OkCnt;
extern u8 Rf433ByteIndex;
void IRorRF_Init(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB,ENABLE);//irout B9,rf433out B7
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_12;////IR的输出GPIOB9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	  //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIO,ENABLE);//rf433in
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	  //上拉
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
//	  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;//rf433in
//	  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
//	  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
//	  GPIO_Init(GPIOB, &GPIO_InitStructure);
//	
	GPIO_ResetBits(GPIOB,GPIO_Pin_9); 
//	Rf433Out=0;
}


void SendRf433One(void)
{
	Rf433Out=1;
	IrTxCnt=43;
	while(IrTxCnt>0);//600us
	Rf433Out=0;
	IrTxCnt=14;
	while(IrTxCnt>0);//200us
}

void SendRf433Zero(void)
{
	Rf433Out=1;
	IrTxCnt=14;
	while(IrTxCnt>0);//200us
	Rf433Out=0;
	IrTxCnt=43;
	while(IrTxCnt>0);//600us
}

void SendRf433Byte(u8 byte)
{
  u8 i;
	
	for(i=0;i<8;i++)
	{
		if(byte&0x01)
		{
			SendRf433One();
		}
		else
		{
			SendRf433Zero();
		}
		byte=byte>>1;
	}
}

void Send_rf433(void)
{
//	u8 i,*p;
//	
//	p = (u8 *)(&rf433_txdata);
//	
	if(SysSetData.Rf433Send_flag == 1)
	{ 
		SysSetData.Rf433Send_flag=0;
//		ACT_LED_EN;

//		ACT_LED_DIS;
		 RF_StartTx((u8*)(&rf433_txdata),rf433_txdata.Len+12,500);
		ASDataFlag=1;//开启200MS计数 ASDataCnt 
		ASTimes++;
		if(ASTimes>=RF433SEND_MAX){ //发送达到了最大计数就不再发了。
		  memset(&rf433_txdata,0,sizeof(rf433_txdata));
//			g_nrf433Count--;
			ASDataFlag=0;//关闭计数
		}
		ASDataCnt=(rand()%5)*100;
	}
}

void Rf433TxSocket(void)
{
	rf433_txdata.SoucreAddr=SysSetData.SysAddr;
	rf433_txdata.Type=0x01;
}

u8 GetCrcCheck(u8 *p,u8 len)
{
	u8 i,crc=0;
	for(i=0;i<len;i++)
	{
		crc += *p++;
	}
	return crc;
}

static int scnt;
u8 tR_Buff[255];
void DisCode_Ir(u8 *p,u8 len)
{
	u8 K1,K2,K3,pv;
	u8 i;
	
	for(i=0;i<len;i++)
	{
	   tR_Buff[i]=*(p+i);
	}
	
	K1 = tR_Buff[0];
	K2 = len-1;
	
	tR_Buff[0]=K2;  //恢复长度
	
	for(K3=1;K3<len;K3++) 
  {
		pv=(pwda[K3]+pwdb[K2]-pwdc[K1]) % 256;
		tR_Buff[K3]=tR_Buff[K3]^pv;
  }
	
	if(tR_Buff[3])   //特殊编码跳变处理
	{
		scnt+=1;      //scnt为全局变量
		
		if(scnt % 2)	//决定是否跳变
		{
			if(tR_Buff[3]!=255)
			{
			  tR_Buff[tR_Buff[3]]=tR_Buff[tR_Buff[3]]^tR_Buff[4];
			}
			else	//处理PHILIPS跳变
			{
				K1=tR_Buff[26];
				K2=tR_Buff[27];
				K3=tR_Buff[28];
				tR_Buff[26]=tR_Buff[29];
				tR_Buff[27]=tR_Buff[30];
				tR_Buff[28]=tR_Buff[31];
				tR_Buff[29]=tR_Buff[32];
				tR_Buff[30]=K1;
				tR_Buff[31]=K2;
				tR_Buff[32]=K3;
			}  
		}	
	}
}

void New_IR_Send(void)//装载码库数据
{
	u8 cnt;
	u8 chr;
	static volatile u16 Bytet=0;
	static volatile u8 Bitt=0;
	u16 bytes = 0;

	u8 L_M_SB =0;
	u8 Code_Rule=0;
	u8 Order_Carrier_0 =0;
	u8 Order_Carrier_1 =0;
	u16  bits = 0;
	u16  i=0;
	IrStudy_DATA IrStudyData;
	
	cnt = 15;
	chr= tR_Buff[cnt];
	IrStudyData.Data0HCnt = (((tR_Buff[7]&0x7F)<<8)+ tR_Buff[8]);
	IrStudyData.Data0HCnt /= 50;
	IrStudyData.Data0LCnt = (((tR_Buff[9]&0x7F)<<8)+ tR_Buff[10]);
	IrStudyData.Data0LCnt /= 50;
	IrStudyData.Data1HCnt = (((tR_Buff[11]&0x7F)<<8)+ tR_Buff[12]);
	IrStudyData.Data1HCnt /= 50;
	IrStudyData.Data1LCnt = (((tR_Buff[13]&0x7F)<<8)+ tR_Buff[14]);
	IrStudyData.Data1LCnt /= 50;
	L_M_SB = tR_Buff[5] & 0x0f;
	Code_Rule = tR_Buff[5]&0xf0;
	Order_Carrier_0 = tR_Buff[7] & (0x80);//逻辑0表示，1-先载波后间隔
	Order_Carrier_1 = tR_Buff[11] & (0x80);//逻辑1表示，1-先载波后间隔
	
	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
#if 1
	while ((chr != 0)) //执行红外发送
	{
		IWDG_Feed();
		switch (chr)     //判断是什么类型
		{
			case 0xC1:     //载波时间
				IrTxCnt = ((tR_Buff[cnt+1]<<8)+ tR_Buff[cnt+2]);
			  IrTxCnt /= 50;
				SysSetData.IrTxH_flag=1;//载波使能
				while(IrTxCnt>0){}
				IrOut = 0;
				SysSetData.IrTxH_flag=0;//载波失能
				cnt += 3;    //后面跟2个字节 cnt跳到下一个标签（C1，C2或C3）				
				break;
			case 0xC2:     //间隔时间
				SysSetData.IrTxH_flag=0;//载波失能
			  IrOut=0;
				IrTxCnt = ((tR_Buff[cnt+1]<<16)+(tR_Buff[cnt+2]<<8)+ tR_Buff[cnt+3]);
        IrTxCnt /= 50;
				while(IrTxCnt>0){}
				cnt += 4;    //后面跟3个字节 cnt跳到下一个标签（C1，C2或C3）
				break;
			case 0xC3:     //发送二进制序列
				{
    			bits = (tR_Buff[cnt + 1]<<8) + tR_Buff[cnt + 2];//要发送的bit数
    			for(i=0;i<bits;i++)
    			{
						if(0x00 == L_M_SB)//lsb
						{
							Bytet=i/8;  //字节数
							Bitt = i%8;
						}
						else          //msb
						{
							Bytet=i/8;
							Bitt= 7-(i%8);
						}
						
						if ((tR_Buff[cnt+3+Bytet] & 0x01<<Bitt)==0)//判断发逻辑0还是发逻辑1
						{
							if(0x80 == Order_Carrier_0)
							{
								SysSetData.IrTxH_flag=1;//载波使能
								IrTxCnt=IrStudyData.Data0HCnt;
								while(IrTxCnt>0){}
								IrOut=0;
								SysSetData.IrTxH_flag=0;//载波失能
								IrTxCnt=IrStudyData.Data0LCnt;
								while(IrTxCnt>0){}
							}
							else
							{
								IrOut=0;
								SysSetData.IrTxH_flag=0;//载波失能
								IrTxCnt=IrStudyData.Data0HCnt;
								while(IrTxCnt>0){}
								SysSetData.IrTxH_flag=1;//载波使能
								IrTxCnt=IrStudyData.Data0LCnt;
								while(IrTxCnt>0){}
								IrOut=0;
							}
						}
						else //以下大括号发逻辑1
						{
							if(0x80 == Order_Carrier_1)
							{//先发载波再发间隔
								SysSetData.IrTxH_flag=1;//载波使能
								IrTxCnt=IrStudyData.Data1HCnt;
								while(IrTxCnt>0){}
								IrOut=0;
								SysSetData.IrTxH_flag=0;//载波失能
								IrTxCnt=IrStudyData.Data1LCnt;
								while(IrTxCnt>0){}
							}
							else
							{
								IrOut=0;
								SysSetData.IrTxH_flag=0;//载波失能
								IrTxCnt=IrStudyData.Data1HCnt;
								while(IrTxCnt>0){}
								SysSetData.IrTxH_flag=1;//载波使能
								IrTxCnt=IrStudyData.Data1LCnt;
								while(IrTxCnt>0){}
								IrOut=0;
							}
						}
     		  }
					
					if(0 == Code_Rule)
					{
						SysSetData.IrTxH_flag=1;//载波使能
						IrTxCnt=IrStudyData.Data1HCnt;
						while(IrTxCnt>0){}
						SysSetData.IrTxH_flag=0;//载波失能
						IrOut=0;
          }
					bytes=bits/8;  //bits
					
					if((bits % 8) != 0)
				    	bytes = bytes + 1;   				     
					cnt = cnt + bytes + 3;    //改变cnt寻找下一个标签
				}
				break;
			default:
 				{
					tR_Buff[cnt]=0;//出错退出
 				}
 				break;				
		}
		chr = tR_Buff[cnt];
	}
	
	for(i =0;i<(tR_Buff[0]+1);i++)
	{
		tR_Buff[i] = 0x00;
	}
	#endif
	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM1, DISABLE);
}

void OnSlave(void)
{
//    char str[32];
    
    switch(RF_Process())
    {
    case RF_IDLE:
    {
			SysSetData.Rf433Send_flag=1;
			RF_StartRx(rf433_data,63,INFINITE);
        break;
    }
    
    case RF_RX_DONE:
    {

        rf433_rcvstat=workok;
			  //RF_StartRx(rf433_data,32,INFINITE);
        break;
    }
    
    case RF_TX_DONE:
    {
 
        rf433_rcvstat=nowork;
			  RF_StartRx(rf433_data,63,INFINITE);
			  ACT_LED_DIS;
        break;
    }
    
    case RF_ERROR:
    {

        SysSetData.Rf433Send_flag=0;
        break;
    }
    
    default:
        break;
    }
}

 static u8 uc_packetID;
void Task_rf433(void)
{
	u8 i;
	IrOrRf_DATA 	* unpacket;
	IrStudy_DATA	*	stp_RFStudy;
//	static u8 OldCmd;
	#ifdef EE_ENABLE
	u16 Tmp[2];
	#endif
  u32 tAddr=0,tAddr1=0;
	
  if(rf433_rcvstat == workok)
  {
		
		rf433_rcvstat = nowork;
		rf433_intstat.flag = 0;
    rf433_intstat.cnt = 0;
		
		ASDataFlag=0;  //补发旗标
    ASTimes=0;     //补发次数10.29
		ASDataCnt=0;

		unpacket		=(IrOrRf_DATA *		)	rf433_data;
		stp_RFStudy	=(IrStudy_DATA	*	)	(&unpacket->Data[0]);
		switch(unpacket->Cmd)//命令类型rf433_data[9]
		{
			case 0x24://查询
					if(SysSetData.bWasAdd==1)//tAddr1
					{

						rf433_txdata.DistAddr=unpacket->SoucreAddr;//tAddr;//主机地址
						rf433_txdata.Cmd=0x80|unpacket->Cmd;//+rf433_data[9];
						rf433_txdata.Id=unpacket->Id;
						rf433_txdata.Len=2;//1字节温度
						rf433_txdata.Id=unpacket->Id;						
						rf433_txdata.Data[0]=(u8)(SysSetData.Tempt/100);
						rf433_txdata.Data[1]=1;
						rf433_txdata.Data[2]=GetCrcCheck((u8 *)&rf433_txdata,rf433_txdata.Len+12);
				    RF_StartTx((u8*)(&rf433_txdata),rf433_txdata.Len+12,500);
//						CMT2300_ForceTX();
//						SysSetData.Rf433Send_flag=1;
//						for(i=0;i<3;i++)
//						{
//							Send_rf433_1();
//							delay_ms(8);
//						}
//						SysSetData.Rf433Send_flag=0;
//            CMT2300_ForceRX();
					}
					break;
			case 0x25://发送红外控制命令
//					if((unpacket->DistAddr==SysSetData.SysAddr)&&(SysSetData.bWasAdd==1))
			   if(SysSetData.bWasAdd==1)
					{
						rf433_txdata.DistAddr	=	unpacket->SoucreAddr;//tAddr;//主机地址
						rf433_txdata.Cmd			=	0x80|unpacket->Cmd;//+rf433_data[9];
						rf433_txdata.Id				=	unpacket->Id;
						rf433_txdata.Len			=	1;//1字节温度
						rf433_txdata.Id				=	unpacket->Id;
						rf433_txdata.Data[0]	=	(u8)(SysSetData.Tempt/100);
						if( ( uc_packetID != unpacket->Id )	|| ( uc_packetID == 0 ) ){
							uc_packetID = unpacket->Id;
							if(stp_RFStudy->LibCHK == 0x5AA5)  //ir(rf433_data[11]==0xff)&&(rf433_data[12]==0xff)
							{
								SysSetData.Rf433Send_flag = 1;
	//							TIM_Cmd(TIM3, ENABLE);
								switch(stp_RFStudy->Type)//类型rf433_data[13]
								{
									case 0x01: //ir
	//									CMT2300_ForceRX();
										SendIrStudy(stp_RFStudy);
	//								  delay_ms(300);
										break;
									case 0x02: //433
										RF_Init();
										CMT2300_ForceTX();
										for( i = 0; i < 3; i++)
										{
											Send433Study(stp_RFStudy);
											delay_ms(8);
										}
										PWR_ClearFlag(PWR_FLAG_WU);
										RF_Init_FSK(FALSE);
										break;
									default:
										break;
								}
	//							TIM_Cmd(TIM3, DISABLE);
								SysSetData.Rf433Send_flag=0;
							}
							else
							{
	//							TIM_Cmd(TIM3, ENABLE);
								//SysSetData.Rf433Send_flag=1;
								DisCode_Ir(unpacket->Data,unpacket->Len); //解密&rf433_data[11],rf433_data[10]
	//							INTX_DISABLE();
								New_IR_Send();//发送码库数据
	//							INTX_ENABLE();  //在发送码库的时候不得被中断打断
								//SysSetData.Rf433Send_flag=0;
	//							TIM_Cmd(TIM3, DISABLE);
							}
						}
						delay_ms(50);
						Cmt2300_GoStby();
				    Cmt2300_SetResendTime(40);
				    Cmt2300_GoSleep();
									RF_StartTx((u8*)(&rf433_txdata),rf433_txdata.Len+12,500);
//									while(RF_TX_DONE!=RF_Process()){
//										;
//									}						
					}
					break;
			case 0x26://删除分机
//					if(unpacket->DistAddr==SysSetData.SysAddr)
					{
//						Cmt2300_GoTx();
						#ifdef EE_ENABLE
						INTX_DISABLE();
						//擦除主机地址
						Tmp[0] = 0xffff;
						Tmp[1] = 0xffff;
						STMFLASH_Write(FLASH_SAVE_ADDR1,Tmp,2);
						INTX_ENABLE();
						#endif
						SysSetData.MasterAddr = 0;//未分配主机
						SysSetData.bWasAdd = 0;   
						Rf433TxSocket();
						tAddr=rf433_data[3];
						tAddr<<=24;
						tAddr1=rf433_data[2];
						tAddr1<<=16;
						tAddr+=tAddr1;
						tAddr1=0;
						tAddr1=(rf433_data[0]+(rf433_data[1]<<8));
						tAddr+=tAddr1;
						rf433_txdata.DistAddr=tAddr;//主机地址
						rf433_txdata.Cmd=0x80+rf433_data[9];
						rf433_txdata.Id=unpacket->Id;
						rf433_txdata.Len=1;//1字节温度

           RF_StartTx((u8*)(&rf433_txdata),rf433_txdata.Len+12,500);
					}
					break;
			case 0x21://添加分机,回复主机
//					if(unpacket->DistAddr==0)//如果所接受发送目标地址为0,则为添加分机
					{

						Rf433TxSocket();
						rf433_txdata.DistAddr=unpacket->SoucreAddr;//tAddr1;//主机地址
						rf433_txdata.Cmd=0x80|unpacket->Cmd;//+rf433_data[9];
						rf433_txdata.Id=unpacket->Id;
						rf433_txdata.Len=1;//1字节温度
						rf433_txdata.Id=unpacket->Id;
//						memcpy(rf433_txdata.Data,)
//						delay_ms(200);
						Cmt2300_GoStby();
				    Cmt2300_SetResendTime(10);
				    Cmt2300_GoSleep();
						RF_StartTx((u8*)(&rf433_txdata),rf433_txdata.Len+12,500);

					}
					break;
			case 0x23://确认,无需回复
//					if(unpacket->DistAddr==SysSetData.SysAddr) 已有sycword过滤
					{
				    SysSetData.Rf433Send_flag=0;
						ASDataFlag=0;//关闭计数
						if(SysSetData.key_press==1){//如果是添加的状态
							SysSetData.key_press=0;
							SysSetData.bWasAdd=1;
							Cmt2300_GoStby();
							Cmt2300_SetSynWord(Cmt2300_SYN_LEN,SysSetData.SysAddr);
							//Cmt2300_SetResendTime(40);
							Cmt2300_GoSleep();
						}
						SysSetData.MasterAddr=unpacket->SoucreAddr;//tAddr1;//主机地址
						#ifdef EE_ENABLE
						INTX_DISABLE();
						Tmp[0] = (u16)(unpacket->SoucreAddr&0x0000ffff);
						Tmp[1] = (u16)((unpacket->SoucreAddr&0xffff0000)>>16);
						STMFLASH_Write(FLASH_SAVE_ADDR1,Tmp,2);//存储主机地址
						INTX_ENABLE();
						#endif
						SysSetData.tim.sec=unpacket->Data[0];//rf433_data[11];//赋予时间
						SysSetData.tim.min=unpacket->Data[1];//rf433_data[12];
						SysSetData.tim.hr=unpacket->Data[2];//rf433_data[13];
						SysSetData.tim.date=unpacket->Data[3];//rf433_data[14];//星期
					}
					break;		
			default:
				  RF_StartRx(rf433_data,63,INFINITE);//如果命令是错的那么继续接收或者开启低功耗
			    CMT2300A_LowpowerMode(TRUE);
					break;
		}	
		Rf433ByteIndex=0;
		rf433_rcvstat = nowork;
  }
}

void SendIrOne(IrStudy_DATA * stp_studyData)
{
	SysSetData.IrTxH_flag = 1;    //1高电平
//	IrTxCnt = unpacket433->Data[9];//rf433_data[20];
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[8];//rf433_data[19];
	IrTxCnt = stp_studyData->Data1HCnt;
	while(IrTxCnt > 0);
	
//	IrTxCnt = unpacket433->Data[11];//rf433_data[22];     //1低电平
//	IrTxCnt <<= 8;
//	IrTxCnt +=unpacket433->Data[10];// rf433_data[21];
	IrTxCnt = stp_studyData->Data1LCnt;
	SysSetData.IrTxH_flag = 0;    
	IrOut=0;
	while(IrTxCnt > 0);
}

void SendIrZero(IrStudy_DATA * stp_studyData)
{
	SysSetData.IrTxH_flag = 1;    //0高电平
//	IrTxCnt =unpacket433->Data[13];// rf433_data[24];
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[12];//rf433_data[23];
	IrTxCnt = stp_studyData->Data0HCnt;
	while(IrTxCnt > 0);
//	
//	IrTxCnt = unpacket433->Data[15];//rf433_data[26];     //0低电平
//	IrTxCnt <<= 8;
//	IrTxCnt +=unpacket433->Data[14];// rf433_data[25];
	IrTxCnt = stp_studyData->Data0LCnt;
	SysSetData.IrTxH_flag = 0;    
	IrOut=0;
	while(IrTxCnt > 0);
}

void SendIrByte(u8 byte,IrStudy_DATA * stp_studyData)
{
	u8 i;
	
	for(i=0;i<8;i++)
	{
		if(byte&0x80)
		{
			SendIrOne(stp_studyData);
		}
		else
		{
			SendIrZero(stp_studyData);
		}
		byte=byte<<1;
	}
}

void SendIrStudy(IrStudy_DATA * stp_studyData)
{
	u8 i;//,tmp;//*p,
	
//	p=(u8 *)(&unpacket433->Data[17]);//(&rf433_data[28]);
	TIM_Cmd(TIM3, ENABLE);//14us 做IR载波
	TIM_Cmd(TIM1, ENABLE); //50us 做脉冲长度计算时基
	SysSetData.IrTxH_flag = 1;    //头高电平
//	IrTxCnt = unpacket433->Data[5];//rf433_data[16];
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[4];//rf433_data[15];
	IrTxCnt = stp_studyData->HeadHCnt;
	while(IrTxCnt > 0);
	
//	IrTxCnt = unpacket433->Data[7];//rf433_data[18];     //头低电平
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[6];//rf433_data[17];
	IrTxCnt = stp_studyData->HeadLCnt;
	SysSetData.IrTxH_flag = 0;    
	IrOut=0;
	while(IrTxCnt > 0);
	IWDG_Feed();
	for(i = 0;i < stp_studyData->Len;i++)  //字节数rf433_data[27]//2018-11-20 unpacket433->Data[16]
	{
		//SendIrByte(*(p+i),unpacket433);
		SendIrByte(stp_studyData->Data[i],stp_studyData); //2018-11-20unpacket433->Data[17+i]
	}
	
////	tmp=rf433_data[10]-rf433_data[27];
//	 tmp=unpacket433->Len-unpacket433->Data[16];
//	switch(tmp)
//	{
//		case 2:
//			IrTxCnt = unpacket433->Data[18+unpacket433->Data[16]];//rf433_data[29+rf433_data[27]];
//			IrTxCnt <<= 8;
//			IrTxCnt +=unpacket433->Data[17+unpacket433->Data[16]];// rf433_data[28+rf433_data[27]];
//		  SysSetData.IrTxH_flag = 1;    //头高电平
//			while(IrTxCnt > 0){}
//		  SysSetData.IrTxH_flag = 0;    
//			IrOut=0;
//			IWDG_Feed();
//			break;
//		case 26:
//			IrTxCnt = unpacket433->Data[18+unpacket433->Data[16]];//rf433_data[29+rf433_data[27]];
//			IrTxCnt <<= 8;
//			IrTxCnt += unpacket433->Data[17+unpacket433->Data[16]];//rf433_data[28+rf433_data[27]];
//		  SysSetData.IrTxH_flag = 1;    //头高电平
//			while(IrTxCnt > 0){}
//		  IrTxCnt = unpacket433->Data[20+unpacket433->Data[16]];//rf433_data[31+rf433_data[27]];     //头低电平
//			IrTxCnt <<= 8;
//			IrTxCnt += unpacket433->Data[19+unpacket433->Data[16]];//rf433_data[30+rf433_data[27]];
//			SysSetData.IrTxH_flag = 0;    
//			IrOut=0;
//			while(IrTxCnt > 0);
//			SysSetData.IrTxH_flag = 1;    //头高电平
//			IrTxCnt = unpacket433->Data[22+unpacket433->Data[16]];//rf433_data[33+rf433_data[27]];
//			IrTxCnt <<= 8;
//			IrTxCnt += unpacket433->Data[21+unpacket433->Data[16]];//rf433_data[32+rf433_data[27]];
//			while(IrTxCnt > 0);
//			IrTxCnt = unpacket433->Data[24+unpacket433->Data[16]];//rf433_data[35+rf433_data[27]];     //头低电平
//			IrTxCnt <<= 8;
//			IrTxCnt += unpacket433->Data[23+unpacket433->Data[16]];//rf433_data[34+rf433_data[27]];
//			SysSetData.IrTxH_flag = 0;    
//			IrOut=0;
//			while(IrTxCnt > 0);
//			IWDG_Feed();
//			break;
//		case 24:
//			IrTxCnt = unpacket433->Data[18+unpacket433->Data[16]];//rf433_data[29+rf433_data[27]];     //头低电平
//			IrTxCnt <<= 8;
//			IrTxCnt += unpacket433->Data[17+unpacket433->Data[16]];//rf433_data[28+rf433_data[27]];
//			SysSetData.IrTxH_flag = 0;    
//			IrOut=0;
//			while(IrTxCnt > 0);
//			SysSetData.IrTxH_flag = 1;    //头高电平
//			IrTxCnt = unpacket433->Data[20+unpacket433->Data[16]];//rf433_data[31+rf433_data[27]];
//			IrTxCnt <<= 8;
//			IrTxCnt += unpacket433->Data[19+unpacket433->Data[16]];//rf433_data[30+rf433_data[27]];
//			while(IrTxCnt > 0);
//			IrTxCnt = unpacket433->Data[22+unpacket433->Data[16]];//rf433_data[33+rf433_data[27]];     //头低电平
//			IrTxCnt <<= 8;
//			IrTxCnt += unpacket433->Data[21+unpacket433->Data[16]];//rf433_data[32+rf433_data[27]];
//			SysSetData.IrTxH_flag = 0;    
//			IrOut=0;
//			while(IrTxCnt > 0){}
//		  IWDG_Feed();
//			break;
//		default:
//			break;
//	}
	IrOut=0;
	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM1, DISABLE);
}

void Send433One(IrStudy_DATA * stp_studyData)
{
	Rf433Out = 1;
//	IrTxCnt = unpacket433->Data[9];//rf433_data[20];
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[8];//rf433_data[19]; //1高电平
	IrTxCnt = stp_studyData->Data1HCnt;
	while(IrTxCnt > 0);
	
	Rf433Out = 0;
//	IrTxCnt = unpacket433->Data[11];//rf433_data[22];     //1低电平
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[10];//rf433_data[21];
	IrTxCnt = stp_studyData->Data1LCnt;
	while(IrTxCnt > 0);
}

void Send433Zero(IrStudy_DATA * stp_studyData)
{
	Rf433Out = 1;
//	IrTxCnt = unpacket433->Data[13];//rf433_data[24];  //0低电平
//	IrTxCnt <<= 8;
	
//	IrTxCnt += unpacket433->Data[12];//rf433_data[23];
	IrTxCnt = stp_studyData->Data0HCnt;
	while(IrTxCnt > 0);
	
	Rf433Out = 0;
//	IrTxCnt = unpacket433->Data[15];//rf433_data[26];     //0低电平
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[14];//rf433_data[25];
	IrTxCnt = stp_studyData->Data0LCnt;
	while(IrTxCnt > 0);
}

void Send433Byte(u8 byte,IrStudy_DATA * stp_studyData)
{
	u8 i;
	
	for(i=0;i<8;i++)
	{
		if(byte&0x80)
		{
			Send433One(stp_studyData);
		}
		else
		{
			Send433Zero(stp_studyData);
		}
		byte=byte<<1;
	}
}

void Send433Study(IrStudy_DATA * stp_studyData)
{
	u8 i,*p;
	
	CMT2300_ForceTX();
//	TIM_Cmd(TIM3, ENABLE); //14us 做IR载波
	TIM_Cmd(TIM1, ENABLE); //50us 做脉冲长度计算时基
	
//	p=(u8 *)(&unpacket433->Data[17]);//(&rf433_data[28]);
	p = &stp_studyData->Data[0];
	
	Rf433Out = 1;
//	IrTxCnt = unpacket433->Data[5];//rf433_data[16]; //头高电平
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[4];//rf433_data[15];
	IrTxCnt = stp_studyData->HeadHCnt;
	while(IrTxCnt > 0);
	
	Rf433Out = 0;
//	IrTxCnt = unpacket433->Data[7];//rf433_data[18];     //头低电平
//	IrTxCnt <<= 8;
//	IrTxCnt += unpacket433->Data[6];//rf433_data[17];
	IrTxCnt = stp_studyData->HeadLCnt;
	while(IrTxCnt > 0);
//	IWDG_Feed();

	for(i=0;i< stp_studyData->Len;i++)  //字节数rf433_data[27];unpacket433->Data[16]
	{
		Send433Byte(*(p+i),stp_studyData);
		IWDG_Feed();
	}
//	CMT2300_ForceRX();
//	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM1, DISABLE);
}

void Send_rf433_1(void)
{
	u8 i,*p;
  CMT2300_ForceTX();
	p = (u8 *)(&rf433_txdata);
	
	ACT_LED_EN;
	//头码
	Rf433Out = 1;
	IrTxCnt = 393;//5500/14
	while(IrTxCnt > 0);
	Rf433Out = 0;
	IrTxCnt = 43;//600/14
	while(IrTxCnt > 0);
	
	for(i=0;i<(rf433_txdata.Len+12);i++)
	{
		SendRf433Byte(*p);
		p++;
	}
	Rf433Out = 0;
	IWDG_Feed();
	ACT_LED_DIS;
//  CMT2300_ForceRX();
}

