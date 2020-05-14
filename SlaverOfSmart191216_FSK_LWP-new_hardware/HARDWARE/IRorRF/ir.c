#include "main.h"
u8 IRtemp =0;   //temp�Ƕ����ڱ��ļ��ı��������ں����õ��ⲿ����T0_Flag����
                           //�ñ��ļ��ı���ȥ�����ⲿ�ļ�����
u16  IR[IR_ARRAY];
extern volatile u16 T0_Flag;   //���������ⲿ�ļ�����

//add for IR send,2014-10-23
volatile u8 module[255]={0x29,0x00,0x00,0x00,0x00,0x00,0x26,0x82,0x54,0x02,0x4E,0x82,0x54,0x06,0x9A, \
								0xC1,0x23,0x00,0xC2,0x00,0x11,0x8E,0xC3,0x00,0x23,0x49,0x08,0x20,0x50,0x02,0xC2,0x00,0x4E,0x34,\
								0xC3,0x00,0x20,0x01,0x20,0x00,0xD0,0x00};  //0x06 0x33 -- 0x26 0xC3

static volatile u8 Bitt=0;
static volatile u8 Module_Date=0;
static volatile u8 Module_Date_Bit=0;
static volatile u8 Order_Carrier_0 =0;
static volatile u8 Order_Carrier_1 =0;
static volatile u8 Code_Rule=0;
static volatile u8 L_M_SB =0;
static volatile u8 cnt = 15;
static volatile u8 chr= 0;
static volatile u16 bytes = 0;
static volatile u16  bits = 0;
static volatile u16  i=0;
u16  Timing_value=0;
static volatile u16 Bytet=0;
static volatile u16  High_T_0=0;
static volatile u16  Low_T_0 =0;
static volatile u16  High_T_1=0;
static volatile u16  Low_T_1=0;

/**************************************
** ��������: void ir_record(void)
** ��������: �����ж�ѧϰ
** ��ڲ���: ��
** ���ڲ���: ��
****************************************/
void ir_record(void)
{
	u8 ir_end=6;
	u16 i;
	u8 one_or_zero=0;
    
	UNLOCKREG();
	TIMER1->TCMPR = TCNT_COUNT-300;

	//��������Ϊ����
	DrvGPIO_Open(E_GPB,14, E_IO_INPUT);   //IR_input//����PB14ģʽ
	for(i=0;i<300;i++)	//��ջ�����z
	{
		IR[i]=0xFFFF;
	}
	UNLOCKREG(); 
	WDT->WTCR.WTR = 1;  //ι��z
#if 1     
	/*���Ź���ʱ������ѧϰ�ź�*/
	while(1)
	{
		if(DrvGPIO_GetBit(E_GPB,14))
		{
			IRtemp = 1;
                        /*-----------------------------*\
    --------------------|               Modify          |--------------------------
                        |           2013 - 5 - 14       |
                        \*-----------------------------*/
			if(T0_Flag<50)    //��ʱ5s����5s�ڲ���������Ź���λ
			{
				UNLOCKREG(); 
				WDT->WTCR.WTR = 1;
			}                   
    }
    else
		{
			IRtemp = 0;
			T0_Flag = 0;
			break;
		}
  }
#endif        
        
	for(i=0; i<300 && ir_end; i+=2)
	{
		UNLOCKREG(); 
		WDT->WTCR.WTR = 1;
		
		if(one_or_zero==0)
		{
			TIMER1->TCMPR = TCNT_COUNT-i;
			one_or_zero=1;
			while(!DrvGPIO_GetBit(E_GPB,14))
			{
				if(TIMER1->TDR > TCNT_COUNT)
				{
					ir_end--;
					one_or_zero=0;
					break;
				}
			}
            /*------------------------------------------------*\
            |                                                  |
    --------| �˴���IR�����TDR�Ĵ����ĳ��Ȳ�ƥ�䣬���ܻ�������|---------
            |                                                  |
            \**************************************************/        
			IR[i] = TIMER1->TDR; 
		} //0�ĳ��� 
		
		if(one_or_zero==1)
		{
			TIMER1->TCMPR = TCNT_COUNT+i;
			one_or_zero=0;
			while(DrvGPIO_GetBit(E_GPB,14))
				if(TIMER1->TDR > TCNT_COUNT)
				{
					ir_end--;
					one_or_zero=1;
					break;
				}
			IR[i+1] = TIMER1->TDR;  
		}//1�ĳ���
	}
                        /*-----------------------------*\
    --------------------|               Modify          |--------------------------
                        |           2013 - 5 - 16       |
                        \*-----------------------------*/
  DrvGPIO_SetBit(E_GPE,5);
}

/**************************************
** ��������: void ir_send(void)
** ��������: ���ⷢ��
** ��ڲ���: ��
** ���ڲ���: ��
****************************************/
void ir_send(void)
{
	u8 ir_end = 6;
	u16 i;
	u8 one_or_zero = 0;

	for(i=0; i<300 && ir_end; i+=2)
  {
		UNLOCKREG(); 
		WDT->WTCR.WTR = 1;
		
    /*0�ĳ���*/
    if(one_or_zero == 0)
    {
			outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
			outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
			outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
			
			TIMER1->TCMPR = TCNT_COUNT-i; 
			one_or_zero = 1;
			if(IR[i] > TCNT_COUNT)
			{
				ir_end--;
				one_or_zero = 0;
			}
			while(TIMER1->TDR < IR[i]);
    }
		
    /*1�ĳ���*/
    if(one_or_zero == 1)
    {
			outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));
			outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
			outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
			DrvGPIO_Open(E_GPB,11, E_IO_INPUT);   //IR_output
			TIMER1->TCMPR = TCNT_COUNT+i;
			one_or_zero = 0;
			
			if(IR[i+1] > TCNT_COUNT)
			{
				ir_end--;
				one_or_zero = 1;
			}
			while(TIMER1->TDR < IR[i+1]);
		}
	}
	outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));
	outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
	outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
}

/**** Used IR code data base send IR signal****/
//����������ݣ���װ��
void New_IR_Send(void)
{
	cnt = 15;
	chr= module[cnt];
	High_T_0 = ((module[7]&0x7F)<<8)+ module[8];//����0�ߵ�ƽ
	Low_T_0 =  ((module[9]&0x7F)<<8)+ module[10];//����0�͵�ƽ
	High_T_1 = ((module[11]&0x7F)<<8)+ module[12];//����1�ߵ�ƽ
  Low_T_1 =  ((module[13]&0x7F)<<8)+ module[14];//����1�͵�ƽ
	L_M_SB = module[5] & 0x0f;
	Code_Rule = module[5]&0xF0;
	Order_Carrier_0 = module[7] & (0x80);//1:���ز����� 0:�ȼ�����ز�
	Order_Carrier_1 = module[11] &(0x80);//1:���ز����� 0:�ȼ�����ز�

#if 1
	while ((chr != 0)) 
	{
		switch (chr) // �ж���ʲô����
		{
			case 0xC1:      //�ز�ʱ��-15
					Timing_value = (module[cnt+1]<<8)+ module[cnt+2]; //16-17
					outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
					outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
					outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
					DrvSYS_Delay(Timing_value);
					outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
					outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
					outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
					cnt += 3; // �����2���ֽ� cnt������һ����ǩ��C1��C2��C3��				
					break;
			case 0xC2:      //���ʱ��
					outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
					outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
					outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
					Timing_value = (module[cnt+1]<<16)+(module[cnt+2]<<8)+ module[cnt+3];				
					DrvSYS_Delay(Timing_value);
					cnt += 4; // �����3���ֽ� cnt������һ����ǩ��C1��C2��C3��
					break;
			case 0xC3:  //���Ͷ���������
					bits = (module[cnt + 1]<<8) + module[cnt + 2];
			
					for (i = 0;i<bits;i++)
					{
						if(0x00 == L_M_SB)
						{
							Bytet=i/8;
							Bitt = i%8;
						}
						else
						{
							Bytet=i/8;
							Bitt= 7-(i%8);
						}
						
						if ((module[cnt+3+Bytet] & 0x01<<Bitt)==0)//�жϷ��߼�0���Ƿ��߼�1
						{
							if(0x80 == Order_Carrier_0)
							{
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
								DrvSYS_Delay(High_T_0);
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
								DrvSYS_Delay(Low_T_0);
							}
							else
							{
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
								DrvSYS_Delay(High_T_0);
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
								DrvSYS_Delay(Low_T_0);
							}
						}
						else //���´����ŷ��߼�1
						{
							if(0x80 == Order_Carrier_1)
							{//�ȷ��ز��ٷ����
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
								DrvSYS_Delay(High_T_1);
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
								DrvSYS_Delay(Low_T_1);
							}
							else
							{
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
								DrvSYS_Delay(High_T_1);
								outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
								outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
								outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
								DrvSYS_Delay(Low_T_1);
							}
					  }
					}
					
					if(0 == Code_Rule)
					{
						outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
						outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
						outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
						DrvSYS_Delay(High_T_1);
						outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
						outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
						outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
          }
 				  bytes=bits/8;  //bits
					if((bits % 8) != 0)
						bytes = bytes + 1;   				     
					cnt = cnt + bytes + 3;// �ı�cntѰ����һ����ǩ
				  break;
			default:
 					module[cnt]=0;//�����˳�
 				  break;				
		}
		chr = module[cnt];
	}
	
	for(i =0;i<(module[0]+1);i++) //��ջ�����
	{
		module[i] = 0x00;
	}
#endif
}

