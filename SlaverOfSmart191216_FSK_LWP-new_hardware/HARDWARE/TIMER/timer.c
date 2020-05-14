#include "sys.h"
#include "timer.h"
u8 Task1msCnt;
extern u16 IrTxCnt;
extern enum rev_func rf433_rcvstat;
extern struct IsIrInt rf433_intstat;
extern SysStruct SysSetData;
extern u8 Rf433SendDlyCnt;
extern u8 Study433MFlag;
extern u16 HeadSpaceCnt;
//=====================================================
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
//=====================================================
void Timer3_Init(u16 arr,u16 psc)//ir
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;                       //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������10Ϊ38Khz
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                    //7200��Ƶ  ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;                 //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);              //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	 
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);              //��ʼ����ʱ��
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                   //����ʱ���ж�
//	TIM_Cmd(TIM3, ENABLE);                                       //ʹ�ܶ�ʱ��
  TIM_Cmd(TIM3, DISABLE); 
	//ʹ��TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              //TIM3_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);							 
}

void TIM3_IRQHandler(void)   //TIM3�ж�-14us
{
//	static u8 ir_flag;
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
//		PAout(2)=~PAout(2);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
		
		if(SysSetData.IrTxH_flag)
		{
//			if(ir_flag)
//			{
//				IrOut=0;
//				ir_flag=0;
//			}
//			else
//			{
//				IrOut=1;
//				ir_flag=1;
//			}
			IrOut =~ IrOut; //��תIR���������ز�
		}
		
//		if(IrTxCnt)
//		{
//		  IrTxCnt--;
//		}
		
//		if(rf433_intstat.flag == 1)//433
//    {
//      rf433_intstat.cnt++;
//      
//			if(rf433_intstat.cnt > 450)
//      {
//        rf433_intstat.cnt = 0;
//        rf433_intstat.flag = 0;
//        rf433_rcvstat = nowork;
//      }
//    }
	}
}			

//==============================================
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2��������Ƿ�Ƶ1��*2������APB1Ϊ8MSYSʱ��8��Ƶ��*2 ���ﶨʱ����Ϊ2MHZ
// 2MHZ����ڶ�ʱ������10��Ƶ����0.2MHZ��ÿ��һ���ж�5us
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��2		 1ms
//==============================================
void Timer2_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;                       //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������10Ϊ1ms
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                  //7200��Ƶ  ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;                 //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);              //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);              //��ʼ����ʱ��
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                   //����ʱ���ж�
	TIM_Cmd(TIM2, ENABLE);                                       //ʹ�ܶ�ʱ��

	//ʹ��TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //TIM2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);							 
}

//================TIM2�ж�=================
void TIM2_IRQHandler(void)   
{
	static u8 time;
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   //���TIMx���жϴ�����λ:TIM �ж�Դ 
	 
		Task1msCnt++;
		
//		PAout(2)=~PAout(2);
		//433
		if(SysSetData.key_press==1){
			if( time++ > 100 ){
				time=0;
				PAout(1)^=1;
			}
	  }
		if(Rf433SendDlyCnt)
		{
			Rf433SendDlyCnt--;
			if(Rf433SendDlyCnt==0)
			{
			  SysSetData.Rf433Send_flag = 1;
			}
		}
	}
}


void Timer1_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;                       //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������10Ϊ1ms
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                  //7200��Ƶ  ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;                 //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);              //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	 
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);              //��ʼ����ʱ��
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);                   //����ʱ���ж�
	TIM_Cmd(TIM1,DISABLE);                                       //ʧ�ܶ�ʱ��
//	TIM_Cmd(TIM1,ENABLE);
	
	//ʹ��TIM1�ж�
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;              //TIM1����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);							 
}

//================TIM1�ж�=================
void TIM1_UP_IRQHandler(void)
{
//	PAout(2)=~PAout(2);
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
//		PAout(2)=~PAout(2);
		 //50us��ʱ�����������ȵļ���
		if(IrTxCnt)
		{
		  IrTxCnt--;
		}
	}
}



