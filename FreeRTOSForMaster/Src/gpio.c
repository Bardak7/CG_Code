/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "delay.h"
#include "sys.h"
#include "radio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "protocol.h"
#include "string.h"
//#include "usart.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

extern TaskHandle_t	ReceiveCMTTaskHandle;
extern TaskHandle_t	StudyTaskHandler;
extern TaskHandle_t	KeyTaskHandle;

//extern volatile u8 	 uc_vgTim2CountMode;
extern volatile u8 	 uc_vgStudyMode;

extern volatile u8 	 uc_vgSendIRFlag;
volatile u8		 uc_vgStudyCount; //学习时间计数
volatile EM_StudyState 	 e_gStudyStat;
ST_RFIRStudy 	 st_gRFStudyData;

volatile static u8	 uc_vsgStudyData;
volatile static u8	 uc_vsgBitIndex;  			//易变动的，且static修饰只能在本文件内被使用 位索引 
volatile static EM_PulseWidth	 s_gEqualPreviousH;	//上一次的高电平是否相等？


extern volatile u8 uc_vgTim1CountRetryRF;  //计算重发延时

vu8	uc_vgTim1KeyCount;
u8	uc_gPressFlag;
/*----------------------------------------------------------------------------*/	
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_AFIO_REMAP_PD01_ENABLE();
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB7  to be LED pin*/
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
  /*Configure GPIO pin : PB10  to be KEY pin*/
	GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*Configure GPIO pin : PB9  to be IRsend pin*/
	GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*Configure GPIO pin : PB14  to be IRin pin*/
	GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*Configure GPIO pin : PB5 PB6  PB5: erase ESP8266 PB6: reset ESP8266*/
	GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=GPIO_PIN_1;            //PA1
  GPIO_InitStruct.Mode=GPIO_MODE_ANALOG;     //模拟
  GPIO_InitStruct.Pull=GPIO_NOPULL;          //不带上下拉
  HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	ESP_ERS_EN;
	ESP_RST_EN;
	MCU_LED_DIS;
//	HAL_NVIC_SetPriority(EXTI15_10_IRQn,7,0);
//	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);   
			//这两个GPIO在sip.h中通过宏定义调用函数实现IO口的初始化。
   
}
/**
  *  GPIOA5 和GPIOA6 的中断，
	*  GPIOA5==>CMT_GPIO1  在FSK下是用于 TX 成功后的CMT2300A中断 OOK 下用于 发送 OOK信号
	*  GPIOA6==>CMT_GPIO2	 在FSK下是用于 RX 成功后的CMT2300A中断 OOK 下用于 接收 OOK信号
  */
void EXTI9_5_IRQHandler(void)
{
//	if(CMT_GPIO1==1){	
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);  //每次进中断直接判断是不是EXIT5中断线产生了中断
//	}											//进入中断如果GPIO2是高电平才去判断是不是产生了接收中断
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
	
}
void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	u16 tmp;
	BaseType_t xHigherPriorityTaskWoken=pdFALSE;
	static u8  s_lSumCycle;//s_lHalfOfL,s_lHalfOfH,
//	delay_xms(100);
	switch(GPIO_Pin)
	{
/*GPIO 5 RFM300H 发送成功脚*/		
		case GPIO_PIN_5 :  //
			 if(uc_vgTim2CountMode == 0){ //当没有学习 也非发送OOK的时候 这个脚的中断表示发送完成
				if(CMT_GPIO1 == 1){
					xTaskNotifyFromISR(ReceiveCMTTaskHandle,CMT_SENDBIT,eSetBits,&xHigherPriorityTaskWoken);
					
				}
			}
			//printf("5中断\r\n");
			break;
/*GPIO 6 RF OOK 输入接口*/			
		case GPIO_PIN_6 :
			//printf("6中断\r\n");    //接收到数据，通过任务通知接收处理任务
			if(uc_vgStudyMode == STUDY_MODE_RF){ //如果是学习状态就开始学习 不应该清CMT2300中断
				switch(e_gStudyStat)
				{
					case waitHeadH:      //如果刚开始学习头码高电平的时候开始计数
						if(RF433IN)
						{
							us_vgPulseCnt=0;
							uc_vsgStudyData=0;
							uc_vsgBitIndex=0;
						}
						else      //如果刚开始学习头码低电平的时候判断高电平是否正常
						{ 
							if((us_vgPulseCnt<300)&&(us_vgPulseCnt>70))//头高在15ms~35ms之间（300*50 ~ 70*50）
							{
								st_gRFStudyData.HeadHCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=waitHeadL;
							}
							else   //如果不正常那么说明还没有开始有无线信号输入进来，重新等待学习
							{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						break;
					case waitHeadL:      //头码低
						if(RF433IN)
						{
							if((us_vgPulseCnt>10) &&(us_vgPulseCnt<(st_gRFStudyData.HeadHCnt/2)))   //如果头码的低正常
							{
								st_gRFStudyData.HeadLCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=waitDataH;
							}
							else
							{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						else
						{
							e_gStudyStat=waitHeadH;
							us_vgPulseCnt=0;
						}
						break;
					case waitDataH:      //数据1高电平
						if(RF433IN==0)
						{
							st_gRFStudyData.Data1HCnt=us_vgPulseCnt;
							us_vgPulseCnt=0;
							e_gStudyStat=waitDataL;
//							s_lHalfOfH=st_gRFStudyData.Data1HCnt/2;
						}
						else
						{
							e_gStudyStat=waitHeadH;
							us_vgPulseCnt=0;
						}
						break;
					case waitDataL:      //数据1低电平
						if(RF433IN)
						{
							st_gRFStudyData.Data1LCnt=us_vgPulseCnt;
							us_vgPulseCnt=0;
							e_gStudyStat=genericWait;
							uc_vsgStudyData=0x01;
							uc_vsgBitIndex=1;          //在引导码后收到了一个完整的高电平和低电平这就是第一位数据
							st_gRFStudyData.Len=0;   //初始化其len和位索引bitIndex
//							s_lHalfOfL=st_gRFStudyData.Data1LCnt/2;
							s_lSumCycle=st_gRFStudyData.Data1LCnt+st_gRFStudyData.Data1HCnt;
						}
						else
						{
							e_gStudyStat=waitHeadH;
							us_vgPulseCnt=0;
							// It should not be reach here;
						}
						break;
					case genericWait:      //第二位往后
						if(RF433IN==0)  //再次判断高电平
						{
							if(st_gRFStudyData.Data1HCnt>us_vgPulseCnt)
							{
								tmp=st_gRFStudyData.Data1HCnt-us_vgPulseCnt;
							}else               
							{							
								tmp=us_vgPulseCnt-st_gRFStudyData.Data1HCnt;
							}
							
							if(tmp<PULSE_TOL) //如果本次高电平与第一次数据高电平他们之间的差值小于Data1HCnt的一半
							{
								s_gEqualPreviousH=equal; //标记一次与之前记录过的高电平时长一致
							}else if(tmp>s_lSumCycle) { //如果本次的高电平时长相差得太大，说明是一个干扰信号 需要重新学习
								
								memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
								e_gStudyStat=waitHeadH;		//学习状态（进程）清零
								us_vgPulseCnt=0;   //脉宽长度计数清零
								uc_vsgBitIndex=0;	//位索引清零
								uc_vsgStudyData=0; //学习的数据清零，
//								portEXIT_CRITICAL();
							}else              //与数据高电平1不一致的短高电平，为数据0高电平
							{  
								s_gEqualPreviousH=unequal;
								if(st_gRFStudyData.Data0HCnt==0)
								{
									st_gRFStudyData.Data0HCnt=us_vgPulseCnt;
								}
							}
							us_vgPulseCnt=0;
						}
						else            //再次判断低电平
						{
							if(st_gRFStudyData.Len==0)//第一个字节以内判断低电平
							{
								if(s_gEqualPreviousH==equal)  //如果之前的高电平时长是一致的
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data1LCnt>us_vgPulseCnt){
										tmp=st_gRFStudyData.Data1LCnt-us_vgPulseCnt;
									}
									else 
										tmp=us_vgPulseCnt-st_gRFStudyData.Data1LCnt;

									if(tmp<PULSE_TOL)
									{
										uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;//高低电平均相同 左移加1 
									}else if(tmp>s_lSumCycle) { //如果本次的低电平时长相差得太大，说明是一个干扰信号 需要重新学习
										memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
										e_gStudyStat=waitHeadH;		//学习状态（进程）清零
										us_vgPulseCnt=0;   //脉宽长度计数清零
										uc_vsgBitIndex=0;	//位索引清零
										uc_vsgStudyData=0; //学习的数据清零，
									}else
									{
										if(st_gRFStudyData.Data0LCnt==0)
										{
											st_gRFStudyData.Data0LCnt=us_vgPulseCnt;//低电平不同 左移不加 1
										}
										uc_vsgStudyData=uc_vsgStudyData<<1;
									}
								}
								else if(s_gEqualPreviousH==unequal)      //如果与之前的高电平不同 那么肯定不是同一个数据 直接记录低电平长度再 左移
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data0LCnt==0)
									{
										st_gRFStudyData.Data0LCnt=us_vgPulseCnt;
									}
									uc_vsgStudyData=uc_vsgStudyData<<1;
								}
							}
							else                  //第二字节及之后判断低电平
							{
								if(s_gEqualPreviousH==equal)
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data1LCnt>us_vgPulseCnt){
										tmp=st_gRFStudyData.Data1LCnt-us_vgPulseCnt;
									}else
										tmp=us_vgPulseCnt-st_gRFStudyData.Data1LCnt;
				
									if(tmp<PULSE_TOL)
									{
										if(uc_vsgBitIndex==0) uc_vsgStudyData=0x01;// 不应该执行到这里
										else uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;
									}else if(tmp>s_lSumCycle) { //如果本次的低电平时长相差得太大且已学习了超过3个字节那可能就是尾巴的一个位了
										if(st_gRFStudyData.Len>3){
											if(uc_vsgBitIndex==7){ //正常情况下应该是把一个完整的字节学完了
												uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;//把最后这个位学完
												e_gStudyStat=studyOK; //标记学习完成
											}
											
										}
									}else //如果即不过大，又不等于之前1的低电平长度
									{
										uc_vsgStudyData=uc_vsgStudyData<<1;      //数据0
										if(st_gRFStudyData.Data0LCnt==0)
										{
											st_gRFStudyData.Data0LCnt=us_vgPulseCnt;//低电平不同
										}
									}
								}
								else if(s_gEqualPreviousH==unequal)  //如果直接高电平期间就不相等，那么这个就是不同的位 标记为0
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data0LCnt==0)
									{
										 st_gRFStudyData.Data0LCnt=us_vgPulseCnt;
									}
									uc_vsgStudyData=uc_vsgStudyData<<1;//数据0
									if(us_vgPulseCnt>(st_gRFStudyData.Data1LCnt+st_gRFStudyData.Data1HCnt)){
										//  且如果低电平的时间大于高低电平之和那么有可能学习完了
										if(st_gRFStudyData.Len>3){
											e_gStudyStat=studyOK;
										}
									}
								}
							}
							us_vgPulseCnt=0;
							uc_vsgBitIndex++;   //先算高再算低，每一次低电平的时候都应该是一个位读取完了，位索引加1
							if(uc_vsgBitIndex>7)   //大于7了表示一个字节读完了 把它放到结构体中的数据里面
							{
								uc_vsgBitIndex = 0;
								st_gRFStudyData.Data[st_gRFStudyData.Len++]=uc_vsgStudyData;
								uc_vsgStudyData=0; //把学习到的数据清零开始学习下一个数据
							}
							if(e_gStudyStat == studyOK){
								uc_vgStudyMode		 = 0; //标记不在学习状态
								uc_vgTim2CountMode = 0;
								e_gStudyStat			 = waitHeadH;  
								__HAL_TIM_DISABLE(&htim2); //学习完成关闭采样基准定时器2
								HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);   //暂时关闭线5，6的中断 在Init FSK后再打开
								/**向任务发送通知并解挂学习任务 在学习任务中去格式化需要返回给客户端的数据**/
//								xTaskResumeFromISR(StudyTaskHandler); //解挂任务
								xTaskNotifyFromISR(StudyTaskHandler,STUDY_SUCCUESS,eSetBits,&xHigherPriorityTaskWoken);
							}
						}
						break;
					default:      
						break;
				}
			}else if(uc_vgTim2CountMode==0){ //如果既不是学习433也不是发送那么就可能是收到FSK数据了
				if(CMT_GPIO2==1) //向任务发送已有数据
				xTaskNotifyFromISR(ReceiveCMTTaskHandle,CMT_RECEIVEBIT,eSetBits,&xHigherPriorityTaskWoken);
			}
			break;
/*GPIO 14 红外接收管*/
/**由于红外接收所接收到的波形是反相的**/
		case GPIO_PIN_14 :
			if(uc_vgStudyMode==STUDY_MODE_IR){
				switch(e_gStudyStat)
				{
					case waitHeadH:      //如果刚开始学习头码高电平的时候开始计数
						if(IR_IN==0)
						{
							us_vgPulseCnt=0;
							uc_vsgStudyData=0;
							uc_vsgBitIndex=0;
						}
						else      //如果刚开始学习头码低电平的时候判断高电平是否正常
						{ 
							if((us_vgPulseCnt<300)&&(us_vgPulseCnt>70))//头高在15ms~3.5ms之间（300*50 ~ 70*50）
							{
								st_gRFStudyData.HeadHCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=waitHeadL;
							}
							else   //如果不正常那么说明还没有开始有无线信号输入进来，重新等待学习
							{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						break;
					case waitHeadL:      //头码低
						if(IR_IN==0)
						{
							if((us_vgPulseCnt>st_gRFStudyData.HeadHCnt/3) &&(us_vgPulseCnt<(st_gRFStudyData.HeadHCnt-(st_gRFStudyData.HeadHCnt/3))))   //如果头码的低正常
							{
								st_gRFStudyData.HeadLCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=waitDataH;
							}
							else
							{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						else
						{
							e_gStudyStat=waitHeadH;
							us_vgPulseCnt=0;
						}
						break;
					case waitDataH:      //数据1高电平
						if(IR_IN)
						{
							if((us_vgPulseCnt>st_gRFStudyData.HeadLCnt/10)&&(us_vgPulseCnt<st_gRFStudyData.HeadLCnt)){ //数据不可大于头低不可小于头低的1/4
								st_gRFStudyData.Data1HCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=waitDataL;
								//s_lHalfOfH=st_gRFStudyData.Data1HCnt/3;
							}else{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						else
						{
							e_gStudyStat=waitHeadH;
							us_vgPulseCnt=0;
						}
						break;
					case waitDataL:      //数据1低电平
						if(IR_IN==0)
						{													//大于高电平的1/5小于他的5倍
							if((us_vgPulseCnt>st_gRFStudyData.Data1HCnt/5)&&(us_vgPulseCnt<st_gRFStudyData.Data1HCnt*5)){
								st_gRFStudyData.Data1LCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=genericWait;
								uc_vsgStudyData=0x01;
								uc_vsgBitIndex=1;          //在引导码后收到了一个完整的高电平和低电平这就是第一位数据
								st_gRFStudyData.Len=0;   //初始化其len和位索引bitIndex
	//							s_lHalfOfL=st_gRFStudyData.Data1LCnt/2;
//								s_lSumCycle=st_gRFStudyData.Data1LCnt+st_gRFStudyData.Data1HCnt;
							}else{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						else
						{
							e_gStudyStat=waitHeadH;
							us_vgPulseCnt=0;
							// It should not be reach here;
						}
						break;
					case genericWait:      //第二位往后
						if(IR_IN)  //再次高电平
						{
							if((us_vgPulseCnt>st_gRFStudyData.Data1HCnt/5) &&((us_vgPulseCnt<st_gRFStudyData.Data1HCnt*5)) ){
								if(st_gRFStudyData.Data1HCnt>us_vgPulseCnt){
									tmp=st_gRFStudyData.Data1HCnt-us_vgPulseCnt;
								}else{							
									tmp=us_vgPulseCnt-st_gRFStudyData.Data1HCnt;
								}
								
								if(tmp<PULSE_TOL){ //如果小于容差即相等{
									s_gEqualPreviousH=equal; //标记一次与之前记录过的高电平时长一致
								}else if(tmp>st_gRFStudyData.HeadLCnt) { //如果本次的高电平时长相差得太大，说明是一个干扰信号 需要重新学习
									
									memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
									e_gStudyStat=waitHeadH;		//学习状态（进程）清零
									us_vgPulseCnt=0;   //脉宽长度计数清零
									uc_vsgBitIndex=0;	//位索引清零
									uc_vsgStudyData=0; //学习的数据清零，
	//								portEXIT_CRITICAL();
								}else {             //与数据高电平1不一致的短高电平，为数据0高电平  
									s_gEqualPreviousH=unequal;
									if(st_gRFStudyData.Data0HCnt==0)
									{
										st_gRFStudyData.Data0HCnt=us_vgPulseCnt;
									}
								}
								us_vgPulseCnt=0;
							}else{
								if(st_gRFStudyData.Len>3){
									e_gStudyStat=studyOK;
								}else{
									memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
									e_gStudyStat=waitHeadH;
								}
								us_vgPulseCnt=0;
							}
						}
						else            //再次低电平
						{
								if(s_gEqualPreviousH==equal)
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data1LCnt>us_vgPulseCnt){
										tmp=st_gRFStudyData.Data1LCnt-us_vgPulseCnt;
									}else
										tmp=us_vgPulseCnt-st_gRFStudyData.Data1LCnt;
										/*如果H上一次的Data1HCnt是相等的 判断 这次的L是否是相等的*/
									//判断 L 相同与否
									if(tmp<PULSE_TOL) //小于容差即相等
									{
										if(uc_vsgBitIndex==0)
											uc_vsgStudyData=0x01;// 不应该执行到这里
										else
											uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;
									}else if(tmp>st_gRFStudyData.HeadLCnt) { //如果本次的低电平时长相差得太大且已学习了超过3个字节那可能就是尾巴的一个位了
										if(st_gRFStudyData.Len>3){
											if(uc_vsgBitIndex<2){ //正常情况下应该是把一个完整的字节学完了
												uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;//把最后这个位学完
												e_gStudyStat=studyOK; //标记学习完成
											}
										}else{
											uc_vsgStudyData=0;
											uc_vsgBitIndex=0;
											us_vgPulseCnt=0;
											e_gStudyStat=waitHeadH;
											memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
										}
									}else //如果即不过大，又不等于之前1的低电平长度
									{
										uc_vsgStudyData=uc_vsgStudyData<<1;      //数据0
										if(st_gRFStudyData.Data0LCnt==0)
										{
											st_gRFStudyData.Data0LCnt=us_vgPulseCnt;//低电平不同
											st_gRFStudyData.Data0HCnt=st_gRFStudyData.Data1HCnt;
										}
									}
								}
								else if(s_gEqualPreviousH==unequal)  //如果直接高电平期间就不相等，那么这个就是不同的位 标记为0
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data0LCnt==0)
									{
										 st_gRFStudyData.Data0LCnt=us_vgPulseCnt;
									}
									uc_vsgStudyData=uc_vsgStudyData<<1;//数据0
									if(us_vgPulseCnt>st_gRFStudyData.HeadLCnt){
										//  且如果低电平的时间大于高低电平之和那么有可能学习完了
										if(st_gRFStudyData.Len>3){
											e_gStudyStat=studyOK;
										}
									}
								}
								if(us_vgPulseCnt>st_gRFStudyData.HeadHCnt){
									e_gStudyStat=studyOK;
								}
							us_vgPulseCnt=0;
							uc_vsgBitIndex++;   //先算高再算低，每一次低电平的时候都应该是一个位读取完了，位索引加1
							if(uc_vsgBitIndex>7)   //大于7了表示一个字节读完了 把它放到结构体中的数据里面
							{
								uc_vsgBitIndex=0;
								st_gRFStudyData.Data[st_gRFStudyData.Len++]=uc_vsgStudyData;
								uc_vsgStudyData=0; //把学习到的数据清零开始学习下一个数据
							}
						}
						break;
					case studyOK :
						uc_vgStudyMode		 = 0; //标记不在学习状态
						uc_vgTim2CountMode = 0;
            e_gStudyStat=waitHeadH; 
						__HAL_TIM_DISABLE(&htim2); //学习完成关闭采样基准定时器2
//						HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);   //暂时关闭线10~15的中断 下次学习红外再打开(2018-11-24: 由于按键也在这个中断不)
						/**向任务发送通知并解挂学习任务 在学习任务中去格式化需要返回给客户端的数据**/
//								xTaskResumeFromISR(StudyTaskHandler); //解挂任务
						xTaskNotifyFromISR(StudyTaskHandler,STUDY_SUCCUESS,eSetBits,&xHigherPriorityTaskWoken);
						break;
					default:      
						break;
				}
			}
		break;
/*GPIO 10 按键 输入脚*/				
		case  GPIO_PIN_10 :
			if(KEY0 == 0){
				//清零 开始计数
				uc_gPressFlag 		= 1;
				uc_vgTim1KeyCount	=	0;
				
			}else if(KEY0 == 1){
				//关闭计时
				uc_gPressFlag = 0;
				// 擦除（任务通知）
				if(uc_vgTim1KeyCount >= PRESS_LONG){
					
					xTaskNotifyFromISR(KeyTaskHandle,LONG_PRESS_BIT,eSetBits,&xHigherPriorityTaskWoken);
					uc_vgTim1KeyCount = 0;
				}else{	
				//如果时不大于 擦除时间 复位（任务通知）
					if(uc_vgTim1KeyCount > 0)
						xTaskNotifyFromISR(KeyTaskHandle,SHORT_PRESS_BIT,eSetBits,&xHigherPriorityTaskWoken);
						uc_vgTim1KeyCount = 0xFF;
				}
				uc_vgTim1KeyCount = 0;
			}
			break;
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //退出中断检查是否需要任务切换
}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，WKUP按下 WK_UP
////注意此函数有响应优先级,KEY0>KEY1>KEY2>WK_UP!!
//u8 KEY_Scan(u8 mode)
//{
//    static u8 key_up=1;     //按键松开标志
//    if(mode==1)key_up=1;    //支持连按
//    if(key_up&&(KEY0==0))//||KEY1==0||KEY2==0||WK_UP==1))
//    {
//        delay_ms(10);
//        key_up=0;
//        if(KEY0==0)       return 1;//KEY0_PRES;
////        else if(KEY1==0)  return KEY1_PRES;
////        else if(KEY2==0)  return KEY2_PRES;
////        else if(WK_UP==1) return WKUP_PRES;          
//    }else if(KEY0==1)key_up=1;//&&KEY1==1&&KEY2==1&&WK_UP==0)
//    return 0;   //无按键按下
//}

/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
