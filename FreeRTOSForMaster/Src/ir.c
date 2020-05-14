#include "ir.h"
#include "rf433comm.h"
#include "tim.h"
#include "gpio.h"
//������� ��ʱ�����
//#define tR_Buff  uc_gTimCmdArr 

/*������ѧϰ���*/
volatile u8 	 uc_vgSendIRFlag;  //����flag����λ Tim3�Ż�����ⷢ���һ���ز�

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

static int scnt;
static u8 tR_Buff[255]; //�˱������������
/**
  * @brief  DisCode_Ir  ��������������
  * @param 	p: ����ָ��
	*	@param	len:���ݳ���
  * @retval None
  */
void DisCode_Ir(const u8 *p,u8 len)
{
	u8 K1,K2,K3,pv;
	u8 i;
//	uc_gTimCmdArr
	for(i=0;i<len;i++)
	{
	   tR_Buff[i]=*(p+i);
	}
	
	K1 = tR_Buff[0];
	K2 = len-1;
	
	tR_Buff[0]=K2;  //�ָ�����
	
	for(K3=1;K3<len;K3++) 
  {
		pv=(pwda[K3]+pwdb[K2]-pwdc[K1]) % 256;
		tR_Buff[K3]=tR_Buff[K3]^pv;
  }
	
	if(tR_Buff[3])   //����������䴦��
	{
		scnt+=1;      //scntΪȫ�ֱ���
		
		if(scnt % 2)	//�����Ƿ�����
		{
			if(tR_Buff[3]!=255)
			{
			  tR_Buff[tR_Buff[3]]=tR_Buff[tR_Buff[3]]^tR_Buff[4];
			}
			else	//����PHILIPS����
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
/**
  * @brief  ExtractDataFormlib �ӿ���ȡ������������ ȡ������ʱ��Ӧ���Ƕ���us
  * @param 	None
  * @retval None
  */
void SendIRFormlib(void)//װ��������ݲ����ͳ�ȥ
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
	ST_RFIRStudy IrStudyData;
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
	Order_Carrier_0 = tR_Buff[7] & (0x80);//�߼�0��ʾ��1-���ز�����
	Order_Carrier_1 = tR_Buff[11] & (0x80);//�߼�1��ʾ��1-���ز�����
	
	uc_vgTim2CountMode=DECREASE; //����IRѧϰ50us�Ķ�ʱ��2Ϊ�ݼ�������
	__HAL_TIM_ENABLE(&htim3); //����TIM3ֱ�Ӹ��ز�
	__HAL_TIM_ENABLE(&htim2); //Tim2�����峤��50usʱ��
#if 1
	while ((chr != 0)) //ִ�к��ⷢ��
	{
		switch (chr)     //�ж���ʲô����
		{
			case 0xC1:     //�ز�ʱ��
				us_vgPulseCnt = ((tR_Buff[cnt+1]<<8)+ tR_Buff[cnt+2]);
			  us_vgPulseCnt /= 50;
				uc_vgSendIRFlag=1;//�ز�ʹ��
				while(us_vgPulseCnt>0){}
				IR_OUT(0);
				uc_vgSendIRFlag=0;//�ز�ʧ��
				cnt += 3;    //�����2���ֽ� cnt������һ����ǩ��C1��C2��C3��					
				break;
			case 0xC2:     //���ʱ��
				uc_vgSendIRFlag=0;//�ز�ʧ��
			  IR_OUT(0);
				us_vgPulseCnt = ((tR_Buff[cnt+1]<<16)+(tR_Buff[cnt+2]<<8)+ tR_Buff[cnt+3]);
        us_vgPulseCnt /= 50;
				while(us_vgPulseCnt>0){}
				cnt += 4;    //�����3���ֽ� cnt������һ����ǩ��C1��C2��C3��
				break;
			case 0xC3:     //���Ͷ���������
				{
    			bits = (tR_Buff[cnt + 1]<<8) + tR_Buff[cnt + 2];//Ҫ���͵�bit��
    			for(i=0;i<bits;i++)
    			{
						if(0x00 == L_M_SB)//lsb
						{
							Bytet=i/8;  //�ֽ���
							Bitt = i%8;
						}
						else          //msb
						{
							Bytet=i/8;
							Bitt= 7-(i%8);
						}
						
						if ((tR_Buff[cnt+3+Bytet] & 0x01<<Bitt)==0)//�жϷ��߼�0���Ƿ��߼�1
						{
							if(0x80 == Order_Carrier_0)
							{
								uc_vgSendIRFlag=1;//�ز�ʹ��
								us_vgPulseCnt=IrStudyData.Data0HCnt;
								while(us_vgPulseCnt>0){}
								IR_OUT(0);
								uc_vgSendIRFlag=0;//�ز�ʧ��
								us_vgPulseCnt=IrStudyData.Data0LCnt;
								while(us_vgPulseCnt>0){}
							}
							else
							{
								IR_OUT(0);
								uc_vgSendIRFlag=0;//�ز�ʧ��
								us_vgPulseCnt=IrStudyData.Data0HCnt;
								while(us_vgPulseCnt>0){}
								uc_vgSendIRFlag=1;//�ز�ʹ��
								us_vgPulseCnt=IrStudyData.Data0LCnt;
								while(us_vgPulseCnt>0){}
							}
						}
						else //���´����ŷ��߼�1
						{
							if(0x80 == Order_Carrier_1)
							{//�ȷ��ز��ٷ����
								uc_vgSendIRFlag=1;//�ز�ʹ��
								us_vgPulseCnt=IrStudyData.Data1HCnt;
								while(us_vgPulseCnt>0){}
								IR_OUT(0);
								uc_vgSendIRFlag=0;//�ز�ʧ��
								us_vgPulseCnt=IrStudyData.Data1LCnt;
								while(us_vgPulseCnt>0){}
							}
							else
							{
								IR_OUT(0);
								uc_vgSendIRFlag=0;//�ز�ʧ��
								us_vgPulseCnt=IrStudyData.Data1HCnt;
								while(us_vgPulseCnt>0){}
								uc_vgSendIRFlag=1;//�ز�ʹ��
								us_vgPulseCnt=IrStudyData.Data1LCnt;
								while(us_vgPulseCnt>0){}
							}
						}
     		  }
					
					if(0 == Code_Rule)
					{
						uc_vgSendIRFlag=1;//�ز�ʹ��
						us_vgPulseCnt=IrStudyData.Data1HCnt;
						while(us_vgPulseCnt>0){}
						uc_vgSendIRFlag=0;//�ز�ʧ��
						IR_OUT(0);
          }
					bytes=bits/8;  //bits
					
					if((bits % 8) != 0)
				    	bytes = bytes + 1;   				     
					cnt = cnt + bytes + 3;    //�ı�cntѰ����һ����ǩ
				}
				break;
			default:
 				{
					tR_Buff[cnt]=0;//�����˳�
 				}
 				break;				
		}
		chr = tR_Buff[cnt];
	}
	
	for(i =0;i<(tR_Buff[0]+1);i++)
	{
		tR_Buff[i] = 0x00;
	}
	__HAL_TIM_DISABLE(&htim2);  //�ض�ʱ��2
	__HAL_TIM_DISABLE(&htim3);  //�ض�ʱ��3
	uc_vgTim2CountMode=0;						//��Ƿ��ͽ���
	
	uc_vgSendIRFlag=0;            //�������ز�
	IR_OUT(0);
	#endif
}


/**
  * @brief  SendIRByte �ֽڲ���ķ��� ���Զ���ѧϰ�Ľ�����
  * @param 	uc_argByte����Ҫ���͵��ֽ�
  * @retval None
  */
static void SendIRByte(ST_RFIRStudy *stp_argIRStudy,u8 uc_argByte)
{
	u8 i;
	
	for(i=0;i<8;i++)
	{
		if(uc_argByte&0x80)
		{
			uc_vgSendIRFlag=1;
			us_vgPulseCnt=stp_argIRStudy->Data1HCnt;
			while(us_vgPulseCnt>0);
			
			uc_vgSendIRFlag=0;
			IR_OUT(0);
			us_vgPulseCnt=stp_argIRStudy->Data1LCnt;
			while(us_vgPulseCnt>0);
		}
		else
		{
			uc_vgSendIRFlag=1;
			us_vgPulseCnt=stp_argIRStudy->Data0HCnt;
			while(us_vgPulseCnt>0);
			
			uc_vgSendIRFlag=0;
			IR_OUT(0);
			us_vgPulseCnt=stp_argIRStudy->Data0LCnt;
			while(us_vgPulseCnt>0);
		}
		uc_argByte = uc_argByte << 1;
	}
}
/**
  * @brief  IRSendStudy ������ѧϰ���ĺ������� �����
  * @param 	stp_argIRStudy:stp_argIRStudy
  * @retval None
  */
void IRSendStudy(ST_RFIRStudy *stp_argIRStudy)
{
	u8 i;
	uc_vgTim2CountMode=DECREASE; //����IRѧϰ50us�Ķ�ʱ��2Ϊ�ݼ�������
	/*ͷ���*/
	uc_vgSendIRFlag=1;
	us_vgPulseCnt=stp_argIRStudy->HeadHCnt;
	__HAL_TIM_ENABLE(&htim3); //����TIM3ֱ�Ӹ��ز�
	__HAL_TIM_ENABLE(&htim2); //���峤��ʱ��
	while(us_vgPulseCnt>0);
	/*ͷ���*/
	uc_vgSendIRFlag=0;  
	IR_OUT(0);
	us_vgPulseCnt=stp_argIRStudy->HeadLCnt;
	while(us_vgPulseCnt>0);
	
	for(i=0; i< stp_argIRStudy->Len; i++){
		SendIRByte(stp_argIRStudy,stp_argIRStudy->Data[i]);
	}
	__HAL_TIM_DISABLE(&htim2);  //�ض�ʱ��2
	__HAL_TIM_DISABLE(&htim3);  //�ض�ʱ��3
	uc_vgTim2CountMode=0;						//��Ƿ��ͽ���
	
	
	uc_vgSendIRFlag=0;            //�������ز�
	IR_OUT(0);
}