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
volatile u8		 uc_vgStudyCount; //ѧϰʱ�����
volatile EM_StudyState 	 e_gStudyStat;
ST_RFIRStudy 	 st_gRFStudyData;

volatile static u8	 uc_vsgStudyData;
volatile static u8	 uc_vsgBitIndex;  			//�ױ䶯�ģ���static����ֻ���ڱ��ļ��ڱ�ʹ�� λ���� 
volatile static EM_PulseWidth	 s_gEqualPreviousH;	//��һ�εĸߵ�ƽ�Ƿ���ȣ�


extern volatile u8 uc_vgTim1CountRetryRF;  //�����ط���ʱ

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
  GPIO_InitStruct.Mode=GPIO_MODE_ANALOG;     //ģ��
  GPIO_InitStruct.Pull=GPIO_NOPULL;          //����������
  HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	ESP_ERS_EN;
	ESP_RST_EN;
	MCU_LED_DIS;
//	HAL_NVIC_SetPriority(EXTI15_10_IRQn,7,0);
//	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);   
			//������GPIO��sip.h��ͨ���궨����ú���ʵ��IO�ڵĳ�ʼ����
   
}
/**
  *  GPIOA5 ��GPIOA6 ���жϣ�
	*  GPIOA5==>CMT_GPIO1  ��FSK�������� TX �ɹ����CMT2300A�ж� OOK ������ ���� OOK�ź�
	*  GPIOA6==>CMT_GPIO2	 ��FSK�������� RX �ɹ����CMT2300A�ж� OOK ������ ���� OOK�ź�
  */
void EXTI9_5_IRQHandler(void)
{
//	if(CMT_GPIO1==1){	
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);  //ÿ�ν��ж�ֱ���ж��ǲ���EXIT5�ж��߲������ж�
//	}											//�����ж����GPIO2�Ǹߵ�ƽ��ȥ�ж��ǲ��ǲ����˽����ж�
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
/*GPIO 5 RFM300H ���ͳɹ���*/		
		case GPIO_PIN_5 :  //
			 if(uc_vgTim2CountMode == 0){ //��û��ѧϰ Ҳ�Ƿ���OOK��ʱ�� ����ŵ��жϱ�ʾ�������
				if(CMT_GPIO1 == 1){
					xTaskNotifyFromISR(ReceiveCMTTaskHandle,CMT_SENDBIT,eSetBits,&xHigherPriorityTaskWoken);
					
				}
			}
			//printf("5�ж�\r\n");
			break;
/*GPIO 6 RF OOK ����ӿ�*/			
		case GPIO_PIN_6 :
			//printf("6�ж�\r\n");    //���յ����ݣ�ͨ������֪ͨ���մ�������
			if(uc_vgStudyMode == STUDY_MODE_RF){ //�����ѧϰ״̬�Ϳ�ʼѧϰ ��Ӧ����CMT2300�ж�
				switch(e_gStudyStat)
				{
					case waitHeadH:      //����տ�ʼѧϰͷ��ߵ�ƽ��ʱ��ʼ����
						if(RF433IN)
						{
							us_vgPulseCnt=0;
							uc_vsgStudyData=0;
							uc_vsgBitIndex=0;
						}
						else      //����տ�ʼѧϰͷ��͵�ƽ��ʱ���жϸߵ�ƽ�Ƿ�����
						{ 
							if((us_vgPulseCnt<300)&&(us_vgPulseCnt>70))//ͷ����15ms~35ms֮�䣨300*50 ~ 70*50��
							{
								st_gRFStudyData.HeadHCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=waitHeadL;
							}
							else   //�����������ô˵����û�п�ʼ�������ź�������������µȴ�ѧϰ
							{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						break;
					case waitHeadL:      //ͷ���
						if(RF433IN)
						{
							if((us_vgPulseCnt>10) &&(us_vgPulseCnt<(st_gRFStudyData.HeadHCnt/2)))   //���ͷ��ĵ�����
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
					case waitDataH:      //����1�ߵ�ƽ
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
					case waitDataL:      //����1�͵�ƽ
						if(RF433IN)
						{
							st_gRFStudyData.Data1LCnt=us_vgPulseCnt;
							us_vgPulseCnt=0;
							e_gStudyStat=genericWait;
							uc_vsgStudyData=0x01;
							uc_vsgBitIndex=1;          //����������յ���һ�������ĸߵ�ƽ�͵͵�ƽ����ǵ�һλ����
							st_gRFStudyData.Len=0;   //��ʼ����len��λ����bitIndex
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
					case genericWait:      //�ڶ�λ����
						if(RF433IN==0)  //�ٴ��жϸߵ�ƽ
						{
							if(st_gRFStudyData.Data1HCnt>us_vgPulseCnt)
							{
								tmp=st_gRFStudyData.Data1HCnt-us_vgPulseCnt;
							}else               
							{							
								tmp=us_vgPulseCnt-st_gRFStudyData.Data1HCnt;
							}
							
							if(tmp<PULSE_TOL) //������θߵ�ƽ���һ�����ݸߵ�ƽ����֮��Ĳ�ֵС��Data1HCnt��һ��
							{
								s_gEqualPreviousH=equal; //���һ����֮ǰ��¼���ĸߵ�ƽʱ��һ��
							}else if(tmp>s_lSumCycle) { //������εĸߵ�ƽʱ������̫��˵����һ�������ź� ��Ҫ����ѧϰ
								
								memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
								e_gStudyStat=waitHeadH;		//ѧϰ״̬�����̣�����
								us_vgPulseCnt=0;   //�����ȼ�������
								uc_vsgBitIndex=0;	//λ��������
								uc_vsgStudyData=0; //ѧϰ���������㣬
//								portEXIT_CRITICAL();
							}else              //�����ݸߵ�ƽ1��һ�µĶ̸ߵ�ƽ��Ϊ����0�ߵ�ƽ
							{  
								s_gEqualPreviousH=unequal;
								if(st_gRFStudyData.Data0HCnt==0)
								{
									st_gRFStudyData.Data0HCnt=us_vgPulseCnt;
								}
							}
							us_vgPulseCnt=0;
						}
						else            //�ٴ��жϵ͵�ƽ
						{
							if(st_gRFStudyData.Len==0)//��һ���ֽ������жϵ͵�ƽ
							{
								if(s_gEqualPreviousH==equal)  //���֮ǰ�ĸߵ�ƽʱ����һ�µ�
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data1LCnt>us_vgPulseCnt){
										tmp=st_gRFStudyData.Data1LCnt-us_vgPulseCnt;
									}
									else 
										tmp=us_vgPulseCnt-st_gRFStudyData.Data1LCnt;

									if(tmp<PULSE_TOL)
									{
										uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;//�ߵ͵�ƽ����ͬ ���Ƽ�1 
									}else if(tmp>s_lSumCycle) { //������εĵ͵�ƽʱ������̫��˵����һ�������ź� ��Ҫ����ѧϰ
										memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
										e_gStudyStat=waitHeadH;		//ѧϰ״̬�����̣�����
										us_vgPulseCnt=0;   //�����ȼ�������
										uc_vsgBitIndex=0;	//λ��������
										uc_vsgStudyData=0; //ѧϰ���������㣬
									}else
									{
										if(st_gRFStudyData.Data0LCnt==0)
										{
											st_gRFStudyData.Data0LCnt=us_vgPulseCnt;//�͵�ƽ��ͬ ���Ʋ��� 1
										}
										uc_vsgStudyData=uc_vsgStudyData<<1;
									}
								}
								else if(s_gEqualPreviousH==unequal)      //�����֮ǰ�ĸߵ�ƽ��ͬ ��ô�϶�����ͬһ������ ֱ�Ӽ�¼�͵�ƽ������ ����
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data0LCnt==0)
									{
										st_gRFStudyData.Data0LCnt=us_vgPulseCnt;
									}
									uc_vsgStudyData=uc_vsgStudyData<<1;
								}
							}
							else                  //�ڶ��ֽڼ�֮���жϵ͵�ƽ
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
										if(uc_vsgBitIndex==0) uc_vsgStudyData=0x01;// ��Ӧ��ִ�е�����
										else uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;
									}else if(tmp>s_lSumCycle) { //������εĵ͵�ƽʱ������̫������ѧϰ�˳���3���ֽ��ǿ��ܾ���β�͵�һ��λ��
										if(st_gRFStudyData.Len>3){
											if(uc_vsgBitIndex==7){ //���������Ӧ���ǰ�һ���������ֽ�ѧ����
												uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;//��������λѧ��
												e_gStudyStat=studyOK; //���ѧϰ���
											}
											
										}
									}else //������������ֲ�����֮ǰ1�ĵ͵�ƽ����
									{
										uc_vsgStudyData=uc_vsgStudyData<<1;      //����0
										if(st_gRFStudyData.Data0LCnt==0)
										{
											st_gRFStudyData.Data0LCnt=us_vgPulseCnt;//�͵�ƽ��ͬ
										}
									}
								}
								else if(s_gEqualPreviousH==unequal)  //���ֱ�Ӹߵ�ƽ�ڼ�Ͳ���ȣ���ô������ǲ�ͬ��λ ���Ϊ0
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data0LCnt==0)
									{
										 st_gRFStudyData.Data0LCnt=us_vgPulseCnt;
									}
									uc_vsgStudyData=uc_vsgStudyData<<1;//����0
									if(us_vgPulseCnt>(st_gRFStudyData.Data1LCnt+st_gRFStudyData.Data1HCnt)){
										//  ������͵�ƽ��ʱ����ڸߵ͵�ƽ֮����ô�п���ѧϰ����
										if(st_gRFStudyData.Len>3){
											e_gStudyStat=studyOK;
										}
									}
								}
							}
							us_vgPulseCnt=0;
							uc_vsgBitIndex++;   //���������ͣ�ÿһ�ε͵�ƽ��ʱ��Ӧ����һ��λ��ȡ���ˣ�λ������1
							if(uc_vsgBitIndex>7)   //����7�˱�ʾһ���ֽڶ����� �����ŵ��ṹ���е���������
							{
								uc_vsgBitIndex = 0;
								st_gRFStudyData.Data[st_gRFStudyData.Len++]=uc_vsgStudyData;
								uc_vsgStudyData=0; //��ѧϰ�����������㿪ʼѧϰ��һ������
							}
							if(e_gStudyStat == studyOK){
								uc_vgStudyMode		 = 0; //��ǲ���ѧϰ״̬
								uc_vgTim2CountMode = 0;
								e_gStudyStat			 = waitHeadH;  
								__HAL_TIM_DISABLE(&htim2); //ѧϰ��ɹرղ�����׼��ʱ��2
								HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);   //��ʱ�ر���5��6���ж� ��Init FSK���ٴ�
								/**��������֪ͨ�����ѧϰ���� ��ѧϰ������ȥ��ʽ����Ҫ���ظ��ͻ��˵�����**/
//								xTaskResumeFromISR(StudyTaskHandler); //�������
								xTaskNotifyFromISR(StudyTaskHandler,STUDY_SUCCUESS,eSetBits,&xHigherPriorityTaskWoken);
							}
						}
						break;
					default:      
						break;
				}
			}else if(uc_vgTim2CountMode==0){ //����Ȳ���ѧϰ433Ҳ���Ƿ�����ô�Ϳ������յ�FSK������
				if(CMT_GPIO2==1) //����������������
				xTaskNotifyFromISR(ReceiveCMTTaskHandle,CMT_RECEIVEBIT,eSetBits,&xHigherPriorityTaskWoken);
			}
			break;
/*GPIO 14 ������չ�*/
/**���ں�����������յ��Ĳ����Ƿ����**/
		case GPIO_PIN_14 :
			if(uc_vgStudyMode==STUDY_MODE_IR){
				switch(e_gStudyStat)
				{
					case waitHeadH:      //����տ�ʼѧϰͷ��ߵ�ƽ��ʱ��ʼ����
						if(IR_IN==0)
						{
							us_vgPulseCnt=0;
							uc_vsgStudyData=0;
							uc_vsgBitIndex=0;
						}
						else      //����տ�ʼѧϰͷ��͵�ƽ��ʱ���жϸߵ�ƽ�Ƿ�����
						{ 
							if((us_vgPulseCnt<300)&&(us_vgPulseCnt>70))//ͷ����15ms~3.5ms֮�䣨300*50 ~ 70*50��
							{
								st_gRFStudyData.HeadHCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=waitHeadL;
							}
							else   //�����������ô˵����û�п�ʼ�������ź�������������µȴ�ѧϰ
							{
								e_gStudyStat=waitHeadH;
								us_vgPulseCnt=0;
							}
						}
						break;
					case waitHeadL:      //ͷ���
						if(IR_IN==0)
						{
							if((us_vgPulseCnt>st_gRFStudyData.HeadHCnt/3) &&(us_vgPulseCnt<(st_gRFStudyData.HeadHCnt-(st_gRFStudyData.HeadHCnt/3))))   //���ͷ��ĵ�����
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
					case waitDataH:      //����1�ߵ�ƽ
						if(IR_IN)
						{
							if((us_vgPulseCnt>st_gRFStudyData.HeadLCnt/10)&&(us_vgPulseCnt<st_gRFStudyData.HeadLCnt)){ //���ݲ��ɴ���ͷ�Ͳ���С��ͷ�͵�1/4
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
					case waitDataL:      //����1�͵�ƽ
						if(IR_IN==0)
						{													//���ڸߵ�ƽ��1/5С������5��
							if((us_vgPulseCnt>st_gRFStudyData.Data1HCnt/5)&&(us_vgPulseCnt<st_gRFStudyData.Data1HCnt*5)){
								st_gRFStudyData.Data1LCnt=us_vgPulseCnt;
								us_vgPulseCnt=0;
								e_gStudyStat=genericWait;
								uc_vsgStudyData=0x01;
								uc_vsgBitIndex=1;          //����������յ���һ�������ĸߵ�ƽ�͵͵�ƽ����ǵ�һλ����
								st_gRFStudyData.Len=0;   //��ʼ����len��λ����bitIndex
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
					case genericWait:      //�ڶ�λ����
						if(IR_IN)  //�ٴθߵ�ƽ
						{
							if((us_vgPulseCnt>st_gRFStudyData.Data1HCnt/5) &&((us_vgPulseCnt<st_gRFStudyData.Data1HCnt*5)) ){
								if(st_gRFStudyData.Data1HCnt>us_vgPulseCnt){
									tmp=st_gRFStudyData.Data1HCnt-us_vgPulseCnt;
								}else{							
									tmp=us_vgPulseCnt-st_gRFStudyData.Data1HCnt;
								}
								
								if(tmp<PULSE_TOL){ //���С���ݲ���{
									s_gEqualPreviousH=equal; //���һ����֮ǰ��¼���ĸߵ�ƽʱ��һ��
								}else if(tmp>st_gRFStudyData.HeadLCnt) { //������εĸߵ�ƽʱ������̫��˵����һ�������ź� ��Ҫ����ѧϰ
									
									memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
									e_gStudyStat=waitHeadH;		//ѧϰ״̬�����̣�����
									us_vgPulseCnt=0;   //�����ȼ�������
									uc_vsgBitIndex=0;	//λ��������
									uc_vsgStudyData=0; //ѧϰ���������㣬
	//								portEXIT_CRITICAL();
								}else {             //�����ݸߵ�ƽ1��һ�µĶ̸ߵ�ƽ��Ϊ����0�ߵ�ƽ  
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
						else            //�ٴε͵�ƽ
						{
								if(s_gEqualPreviousH==equal)
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data1LCnt>us_vgPulseCnt){
										tmp=st_gRFStudyData.Data1LCnt-us_vgPulseCnt;
									}else
										tmp=us_vgPulseCnt-st_gRFStudyData.Data1LCnt;
										/*���H��һ�ε�Data1HCnt����ȵ� �ж� ��ε�L�Ƿ�����ȵ�*/
									//�ж� L ��ͬ���
									if(tmp<PULSE_TOL) //С���ݲ���
									{
										if(uc_vsgBitIndex==0)
											uc_vsgStudyData=0x01;// ��Ӧ��ִ�е�����
										else
											uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;
									}else if(tmp>st_gRFStudyData.HeadLCnt) { //������εĵ͵�ƽʱ������̫������ѧϰ�˳���3���ֽ��ǿ��ܾ���β�͵�һ��λ��
										if(st_gRFStudyData.Len>3){
											if(uc_vsgBitIndex<2){ //���������Ӧ���ǰ�һ���������ֽ�ѧ����
												uc_vsgStudyData=(uc_vsgStudyData<<1)|0x01;//��������λѧ��
												e_gStudyStat=studyOK; //���ѧϰ���
											}
										}else{
											uc_vsgStudyData=0;
											uc_vsgBitIndex=0;
											us_vgPulseCnt=0;
											e_gStudyStat=waitHeadH;
											memset(&st_gRFStudyData,0,sizeof(ST_RFIRStudy));
										}
									}else //������������ֲ�����֮ǰ1�ĵ͵�ƽ����
									{
										uc_vsgStudyData=uc_vsgStudyData<<1;      //����0
										if(st_gRFStudyData.Data0LCnt==0)
										{
											st_gRFStudyData.Data0LCnt=us_vgPulseCnt;//�͵�ƽ��ͬ
											st_gRFStudyData.Data0HCnt=st_gRFStudyData.Data1HCnt;
										}
									}
								}
								else if(s_gEqualPreviousH==unequal)  //���ֱ�Ӹߵ�ƽ�ڼ�Ͳ���ȣ���ô������ǲ�ͬ��λ ���Ϊ0
								{
									s_gEqualPreviousH=nonuse;
									if(st_gRFStudyData.Data0LCnt==0)
									{
										 st_gRFStudyData.Data0LCnt=us_vgPulseCnt;
									}
									uc_vsgStudyData=uc_vsgStudyData<<1;//����0
									if(us_vgPulseCnt>st_gRFStudyData.HeadLCnt){
										//  ������͵�ƽ��ʱ����ڸߵ͵�ƽ֮����ô�п���ѧϰ����
										if(st_gRFStudyData.Len>3){
											e_gStudyStat=studyOK;
										}
									}
								}
								if(us_vgPulseCnt>st_gRFStudyData.HeadHCnt){
									e_gStudyStat=studyOK;
								}
							us_vgPulseCnt=0;
							uc_vsgBitIndex++;   //���������ͣ�ÿһ�ε͵�ƽ��ʱ��Ӧ����һ��λ��ȡ���ˣ�λ������1
							if(uc_vsgBitIndex>7)   //����7�˱�ʾһ���ֽڶ����� �����ŵ��ṹ���е���������
							{
								uc_vsgBitIndex=0;
								st_gRFStudyData.Data[st_gRFStudyData.Len++]=uc_vsgStudyData;
								uc_vsgStudyData=0; //��ѧϰ�����������㿪ʼѧϰ��һ������
							}
						}
						break;
					case studyOK :
						uc_vgStudyMode		 = 0; //��ǲ���ѧϰ״̬
						uc_vgTim2CountMode = 0;
            e_gStudyStat=waitHeadH; 
						__HAL_TIM_DISABLE(&htim2); //ѧϰ��ɹرղ�����׼��ʱ��2
//						HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);   //��ʱ�ر���10~15���ж� �´�ѧϰ�����ٴ�(2018-11-24: ���ڰ���Ҳ������жϲ�)
						/**��������֪ͨ�����ѧϰ���� ��ѧϰ������ȥ��ʽ����Ҫ���ظ��ͻ��˵�����**/
//								xTaskResumeFromISR(StudyTaskHandler); //�������
						xTaskNotifyFromISR(StudyTaskHandler,STUDY_SUCCUESS,eSetBits,&xHigherPriorityTaskWoken);
						break;
					default:      
						break;
				}
			}
		break;
/*GPIO 10 ���� �����*/				
		case  GPIO_PIN_10 :
			if(KEY0 == 0){
				//���� ��ʼ����
				uc_gPressFlag 		= 1;
				uc_vgTim1KeyCount	=	0;
				
			}else if(KEY0 == 1){
				//�رռ�ʱ
				uc_gPressFlag = 0;
				// ����������֪ͨ��
				if(uc_vgTim1KeyCount >= PRESS_LONG){
					
					xTaskNotifyFromISR(KeyTaskHandle,LONG_PRESS_BIT,eSetBits,&xHigherPriorityTaskWoken);
					uc_vgTim1KeyCount = 0;
				}else{	
				//���ʱ������ ����ʱ�� ��λ������֪ͨ��
					if(uc_vgTim1KeyCount > 0)
						xTaskNotifyFromISR(KeyTaskHandle,SHORT_PRESS_BIT,eSetBits,&xHigherPriorityTaskWoken);
						uc_vgTim1KeyCount = 0xFF;
				}
				uc_vgTim1KeyCount = 0;
			}
			break;
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�˳��жϼ���Ƿ���Ҫ�����л�
}
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��WKUP���� WK_UP
////ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>WK_UP!!
//u8 KEY_Scan(u8 mode)
//{
//    static u8 key_up=1;     //�����ɿ���־
//    if(mode==1)key_up=1;    //֧������
//    if(key_up&&(KEY0==0))//||KEY1==0||KEY2==0||WK_UP==1))
//    {
//        delay_ms(10);
//        key_up=0;
//        if(KEY0==0)       return 1;//KEY0_PRES;
////        else if(KEY1==0)  return KEY1_PRES;
////        else if(KEY2==0)  return KEY2_PRES;
////        else if(WK_UP==1) return WKUP_PRES;          
//    }else if(KEY0==1)key_up=1;//&&KEY1==1&&KEY2==1&&WK_UP==0)
//    return 0;   //�ް�������
//}

/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
