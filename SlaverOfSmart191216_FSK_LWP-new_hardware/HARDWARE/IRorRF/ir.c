#include "main.h"
u8 IRtemp =0;   //temp是定义在本文件的变量，用于和引用的外部变量T0_Flag解锁
                           //用本文件的变量去控制外部文件变量
u16  IR[IR_ARRAY];
extern volatile u16 T0_Flag;   //声明引用外部文件变量

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
** 函数名称: void ir_record(void)
** 功能描述: 红外中断学习
** 入口参数: 无
** 出口参数: 无
****************************************/
void ir_record(void)
{
	u8 ir_end=6;
	u16 i;
	u8 one_or_zero=0;
    
	UNLOCKREG();
	TIMER1->TCMPR = TCNT_COUNT-300;

	//红外设置为输入
	DrvGPIO_Open(E_GPB,14, E_IO_INPUT);   //IR_input//设置PB14模式
	for(i=0;i<300;i++)	//清空缓冲区z
	{
		IR[i]=0xFFFF;
	}
	UNLOCKREG(); 
	WDT->WTCR.WTR = 1;  //喂狗z
#if 1     
	/*看门狗定时检测红外学习信号*/
	while(1)
	{
		if(DrvGPIO_GetBit(E_GPB,14))
		{
			IRtemp = 1;
                        /*-----------------------------*\
    --------------------|               Modify          |--------------------------
                        |           2013 - 5 - 14       |
                        \*-----------------------------*/
			if(T0_Flag<50)    //定时5s，在5s内不会产生看门狗复位
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
    --------| 此处的IR数组和TDR寄存器的长度不匹配，可能会有问题|---------
            |                                                  |
            \**************************************************/        
			IR[i] = TIMER1->TDR; 
		} //0的长度 
		
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
		}//1的长度
	}
                        /*-----------------------------*\
    --------------------|               Modify          |--------------------------
                        |           2013 - 5 - 16       |
                        \*-----------------------------*/
  DrvGPIO_SetBit(E_GPE,5);
}

/**************************************
** 函数名称: void ir_send(void)
** 功能描述: 红外发射
** 入口参数: 无
** 出口参数: 无
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
		
    /*0的长度*/
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
		
    /*1的长度*/
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
//解析码库数据，并装载
void New_IR_Send(void)
{
	cnt = 15;
	chr= module[cnt];
	High_T_0 = ((module[7]&0x7F)<<8)+ module[8];//数据0高电平
	Low_T_0 =  ((module[9]&0x7F)<<8)+ module[10];//数据0低电平
	High_T_1 = ((module[11]&0x7F)<<8)+ module[12];//数据1高电平
  Low_T_1 =  ((module[13]&0x7F)<<8)+ module[14];//数据1低电平
	L_M_SB = module[5] & 0x0f;
	Code_Rule = module[5]&0xF0;
	Order_Carrier_0 = module[7] & (0x80);//1:先载波后间隔 0:先间隔后载波
	Order_Carrier_1 = module[11] &(0x80);//1:先载波后间隔 0:先间隔后载波

#if 1
	while ((chr != 0)) 
	{
		switch (chr) // 判断是什么类型
		{
			case 0xC1:      //载波时间-15
					Timing_value = (module[cnt+1]<<8)+ module[cnt+2]; //16-17
					outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) | (0x1<<11));
					outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) | (0x1<<4));
					outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) | (0x1<<5));
					DrvSYS_Delay(Timing_value);
					outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
					outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
					outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
					cnt += 3; // 后面跟2个字节 cnt跳到下一个标签（C1，C2或C3）				
					break;
			case 0xC2:      //间隔时间
					outpw(&SYS->GPBMFP, inpw(&SYS->GPBMFP) & ~(0x1<<11));	 //used for close pwm5 of 38kHz
					outpw(&SYS->ALTMFP, inpw(&SYS->ALTMFP) & ~(0x1<<4));
					outpw(&SYS->GPEMFP, inpw(&SYS->GPEMFP) & ~(0x1<<5));
					Timing_value = (module[cnt+1]<<16)+(module[cnt+2]<<8)+ module[cnt+3];				
					DrvSYS_Delay(Timing_value);
					cnt += 4; // 后面跟3个字节 cnt跳到下一个标签（C1，C2或C3）
					break;
			case 0xC3:  //发送二进制序列
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
						
						if ((module[cnt+3+Bytet] & 0x01<<Bitt)==0)//判断发逻辑0还是发逻辑1
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
						else //以下大括号发逻辑1
						{
							if(0x80 == Order_Carrier_1)
							{//先发载波再发间隔
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
					cnt = cnt + bytes + 3;// 改变cnt寻找下一个标签
				  break;
			default:
 					module[cnt]=0;//出错退出
 				  break;				
		}
		chr = module[cnt];
	}
	
	for(i =0;i<(module[0]+1);i++) //清空缓冲区
	{
		module[i] = 0x00;
	}
#endif
}

