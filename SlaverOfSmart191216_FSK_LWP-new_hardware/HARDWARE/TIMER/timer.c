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
//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
//=====================================================
void Timer3_Init(u16 arr,u16 psc)//ir
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;                       //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到10为38Khz
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                    //7200分频  设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;                 //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);              //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	 
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);              //初始化定时器
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                   //开定时器中断
//	TIM_Cmd(TIM3, ENABLE);                                       //使能定时器
  TIM_Cmd(TIM3, DISABLE); 
	//使能TIM3中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              //TIM3_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);							 
}

void TIM3_IRQHandler(void)   //TIM3中断-14us
{
//	static u8 ir_flag;
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
//		PAout(2)=~PAout(2);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
		
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
			IrOut =~ IrOut; //翻转IR引脚生成载波
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
//通用定时器中断初始化
//这里时钟选择为APB1的2倍（如果非分频1就*2），而APB1为8MSYS时钟8分频后*2 到达定时器的为2MHZ
// 2MHZ如果在定时器中再10分频就是0.2MHZ，每进一次中断5us
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器2		 1ms
//==============================================
void Timer2_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;                       //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到10为1ms
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                  //7200分频  设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;                 //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);              //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);              //初始化定时器
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                   //开定时器中断
	TIM_Cmd(TIM2, ENABLE);                                       //使能定时器

	//使能TIM2中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //TIM2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);							 
}

//================TIM2中断=================
void TIM2_IRQHandler(void)   
{
	static u8 time;
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   //清除TIMx的中断待处理位:TIM 中断源 
	 
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

	TIM_TimeBaseStructure.TIM_Period = arr;                       //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到10为1ms
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                  //7200分频  设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;                 //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);              //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	 
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);              //初始化定时器
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);                   //开定时器中断
	TIM_Cmd(TIM1,DISABLE);                                       //失能定时器
//	TIM_Cmd(TIM1,ENABLE);
	
	//使能TIM1中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;              //TIM1溢出中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);							 
}

//================TIM1中断=================
void TIM1_UP_IRQHandler(void)
{
//	PAout(2)=~PAout(2);
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
//		PAout(2)=~PAout(2);
		 //50us的时基用做脉冲宽度的计数
		if(IrTxCnt)
		{
		  IrTxCnt--;
		}
	}
}



