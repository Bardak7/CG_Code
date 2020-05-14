/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
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
#include "rtc.h"
#include "protocol.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stmflash.h"
#include <stdio.h>
#include <string.h>
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;
ST_Time  st_gRTCTime;
//  ȫ��ʱ������� �˱�����ʱ���RTCȡ���Ļ����Ǵ�����ȡ����
uint32_t	ul_gUnixTimeStramp = 0;

#define xMINUTE   (60) //1�ֵ�����
#define xHOUR     (60*xMINUTE) //1Сʱ������
#define xDAY			86400U //1�������
#define xYEAR     31536000 //1�������

extern volatile EM_ConState em_gWifiState;
extern TaskHandle_t  SendWifiTaskHandle;
extern TaskHandle_t	KeyTaskHandle;
extern TaskHandle_t	TimeTaskHandle;

extern u8 uc_gTimTaskList2Arr[TIMINGTASK_LIST_LEN_MAX][TIMINGTASK_HEAD_LEN_MAX];
static uint32_t Month[12]={
    xDAY*(0),//01��
    xDAY*(31),//02��
    xDAY*(31+28),//03��
    xDAY*(31+28+31),//04��
    xDAY*(31+28+31+30),
    xDAY*(31+28+31+30+31),
    xDAY*(31+28+31+30+31+30),
    xDAY*(31+28+31+30+31+30+31),
    xDAY*(31+28+31+30+31+30+31+31),
    xDAY*(31+28+31+30+31+30+31+31+30),
    xDAY*(31+28+31+30+31+30+31+31+30+31),
    xDAY*(31+28+31+30+31+30+31+31+30+31+30)
  };

/**
* @brief  CalcMinTimstamp ����������������Ҫִ�е������ʱ��� ���������������5����֮�ھ������Ϊ����
* @param 
* @retval None
*/
void CalcMinTimstamp(void)
{
	u8 uc_cycleID;
	//�������ڵ�ʱ���
	ul_gUnixTimeStramp=RTC_ReadTimeCounter(&hrtc);
	
	RECENT_TASK_UTC = ul_gTimeTaskUnixTimArr[0];
	//ѭ���ҳ��������Ҫִ�е�ֵ
	for(uc_cycleID = 0; uc_cycleID < TIMINGTASK_LIST_LEN_MAX; uc_cycleID++){
		//��������е�ʱ���С�����ڵ�ʱ�� (������ʱ��Ͳ�������������ʱ��С������) ��ô�������Ͳ�Ӧ�ñ�ִ�У��������񲻻�С�����ڣ�
		if(ul_gTimeTaskUnixTimArr[uc_cycleID] < ul_gUnixTimeStramp){
			
			ul_gTimeTaskUnixTimArr[uc_cycleID] = 0;
			if(uc_cycleID == 0){
				RECENT_TASK_UTC = 0; //Ĭ����С���ǵ�1���������һ��������ִ����ô�� RECENT_TASK_UTC һ�����
			}
		}
		
		if((RECENT_TASK_UTC == 0)	&& (ul_gTimeTaskUnixTimArr[uc_cycleID] != 0)){
			RECENT_TASK_UTC = ul_gTimeTaskUnixTimArr[uc_cycleID];
		}
		
		//�����ʱ����� ʱ�����Ϊ���� �����������ʱ�仹ҪС�Ͱ������Ϊ�����
		if((ul_gTimeTaskUnixTimArr[uc_cycleID]!=0) && (ul_gTimeTaskUnixTimArr[uc_cycleID]<RECENT_TASK_UTC)){
			RECENT_TASK_UTC = ul_gTimeTaskUnixTimArr[uc_cycleID];
		}
	}
	//�����������������ڵ�ʱ��֮��С��5���Ӿ���������������ʱ���Ϊ����
// if((RECENT_TASK_UTC - ul_gUnixTimeStramp) < ALARM_CYCLE){
		HAL_RTC_SetAlarm_IT(&hrtc,RECENT_TASK_UTC);
//	}
}	
/**
  * @brief  CalcSpecificPeriodicTask ����ָ����������ִ�е�ʱ���
	*					hrtc �ṹ���е�ʱ�� Ӧ�����ϲ�ȡ���˺����ᱻCalcAllTaskѭ������ ��Ӧ��ѭ����ȡ ����CPUʹ��
  * @param 	id	����Ҫ�����ID
	* @param	tody������ȡ����ֵ
	*								@arg	TRUE�� ���������Ҫִ���Ե���Ķ�ʱ��ִ��
	*								@arg	FALSE: ���������Ҫִ�в����㵱���ִ��ʱ��
  * @retval None
  */
void CalcSpecificPeriodicTask(u8 id,u8 today)
{
	u16 us_lHeadOrStrLen=0;
	u8 uc_period=0;	u8 uc_week=0; u8 uc_dayCycle;
	//ȡ�����Ƶ�string len
	memcpy(&us_lHeadOrStrLen,&uc_gTimTaskList2Arr[id][2],2);  
	//Ƶ�������Ƶĺ��� ƫ�����Ƶĳ��ȼ���һ��ͷ��short��string len ��short=+4;
	uc_period = uc_gTimTaskList2Arr[id][us_lHeadOrStrLen + 4]; 
	
//	if(uc_period & 0x80){ //������λ������ 0 �Ǿ���һ�����ڵ�����
		//���������ȡ�������ʱ��
		Extract_Time(&st_gRTCTime,&uc_gTimTaskList2Arr[id][us_lHeadOrStrLen + 5]); //�ƶ���Ƶ�κ��棬ʱ�� ����
		//�鿴�����ִ��ʱ���Ƿ��ѹ�
		if((st_gRTCTime.Time.Hours < hrtc.Time.Hours)	||
		((st_gRTCTime.Time.Hours == hrtc.Time.Hours) && (st_gRTCTime.Time.Minutes < hrtc.Time.Minutes))){
			today = FALSE;
		}
		uc_period &= (~0x80);  //��7λ��������1�������յ�ÿһ���Ƿ�ִ��
		uc_week = hrtc.DateToUpdate.WeekDay;
		//ѭ��7��鿴������Ҫִ��
		for(uc_dayCycle = 0;uc_dayCycle < 8;uc_dayCycle++)
		{
			if(uc_week > 7) uc_week = 1; //����������������ֱ�Ӹ�ֵ������һ
			if(uc_period & (1<<(uc_week-1))){ //�����Ӧ������ִ�� //����WeekDay-1������ǰ7λ��ʾ���ڼ�����1�Ͳ���������
				//���������ȡ�������ʱ��
//					Extract_Time(&st_gRTCTime,&uc_gTimTaskList2Arr[id][us_lHeadOrStrLen + 5]); //�ƶ���Ƶ�κ��棬ʱ�� ����
				if((uc_dayCycle == 0) && (today == FALSE)){ //���������Ҫִ���Ҳ���Ҫ������� ������
					uc_week++; //���ڼ�һ�� �����ǲ���������Ҫִ��
					continue;
				}
				//������������ý�������ڸ���
				st_gRTCTime.date.Date		=	hrtc.DateToUpdate.Date + uc_dayCycle; //ִ��ʱ�������������������ȥ
				st_gRTCTime.date.Month	=	hrtc.DateToUpdate.Month;
				st_gRTCTime.date.Year		=	hrtc.DateToUpdate.Year;
				
//				if(today == FALSE){
//					st_gRTCTime.date.Date--; //��ȥ���죨�����㵱�죩
//				}
				//�������Ӧ��ʱ���
				ul_gTimeTaskUnixTimArr[id]=RTC_ToUTC(&st_gRTCTime);
				break;
			}
			uc_week++; //���ڼ�һ�� �����ǲ���������Ҫִ��
		}
//	}   	
}
/**
  * @brief  CalcSpecificOrdinaryTask  ����ָ������ͨ���񣨷�������ִ������
	*					ul_gUnixTimeStramp Ӧ�����ϲ�ȡ���˺����ᱻCalcAllTaskѭ������ ��Ӧ��ѭ����ȡul_gUnixTimeStramp ����CPUʹ��
  * @param 
  * @retval None
  */
void CalcSpecificNormalTask(u8 id )
{
//	u32 ul_timestamp = 0;
	u16 us_strLen = 0;
//	u8 uc_period=0;
	memcpy(&us_strLen,&uc_gTimTaskList2Arr[id][2],2);  //ȡ�����Ƶ�string len
//	uc_period = uc_gTimTaskList2Arr[id][us_strLen + 4]; //Ƶ�������Ƶĺ��� ƫ�����Ƶĳ��ȼ���һ��ͷ��short��string len ��short=+4;
//	
//	if((uc_period & 0x80) == 0){ //�������ѭ������
		//���������ȡ�������ʱ��
		Extract_Time(&st_gRTCTime,&uc_gTimTaskList2Arr[id][us_strLen + 5]); //�ƶ���Ƶ�κ��棬ʱ�� ����
		//�����Ӧ��ʱ���
		ul_gTimeTaskUnixTimArr[id]=RTC_ToUTC(&st_gRTCTime);
		
		//�жϱ�������û�л���ִ��
		if(ul_gTimeTaskUnixTimArr[id] < ul_gUnixTimeStramp) {
			if((ul_gUnixTimeStramp - ul_gTimeTaskUnixTimArr[id]) < 30){ //�������չ��˾�ִ������
				/*ִ������*/
				ExecuteTask(id);
				memset(&uc_gTimTaskList2Arr[id][0],0,TIMINGTASK_HEAD_LEN_MAX);
				us_strLen = 0;   //
				STMFLASH_Erase_Word(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * id)); //���ܳ�������
			}
			ul_gTimeTaskUnixTimArr[id] = 0; //���û��ִ�оͲ�ɾ������ֻ���� ����ʱ���
		}
//	}
}

/**
  * @brief  CalcPeriodicTask  �������������´�ִ�е�ʱ��  ������Ӧ��ֻ�ڻ�ȡ������ʱ��� ִֻ��һ��
	*														�ڼ���Ĺ����������������ִ�в�����ʱ�䳬�����˾�ѡ��ִ�л��߷���
  * @param 	None
  * @retval None
  */
void CalcAllTask(void)
{
	u8 uc_idCycle=0; //����Ƿ������������ѭ��
	u16 us_lHeadOrStrLen=0;
	u8 uc_period=0;
	//ȡ����RTCʱʱ�����ת��Ϊʱ�����ڵȡ�����hrtc�ṹ����
	HAL_RTC_GetTimeUser(&hrtc);
	//ȡ��RTCʱ���
	ul_gUnixTimeStramp = RTC_ReadTimeCounter(&hrtc);
	for(uc_idCycle = 0;uc_idCycle < TIMINGTASK_LIST_LEN_MAX;uc_idCycle++){
		memcpy(&us_lHeadOrStrLen,&uc_gTimTaskList2Arr[uc_idCycle][0],2); //ȡ��ͷ������
		if(uc_gTimTaskList2Arr[uc_idCycle][us_lHeadOrStrLen + 2] == 1){ //�ȼ�������״̬�Ƿ��ѿ���
			/*ȡ�����Ƶ�string len*/
			memcpy(&us_lHeadOrStrLen,&uc_gTimTaskList2Arr[uc_idCycle][2],2); 
			//ȡ��ִ��Ƶ��
			uc_period = uc_gTimTaskList2Arr[uc_idCycle][us_lHeadOrStrLen + 4]; //Ƶ�������Ƶĺ��� ƫ�����Ƶĳ��ȼ���һ��ͷ��short��string len ��short=+4;
			
			if(uc_period & 0x80){
				CalcSpecificPeriodicTask(uc_idCycle,TRUE);
			}else{
				CalcSpecificNormalTask(uc_idCycle);
			}
		}
	}
	CalcMinTimstamp(); //��ʼ����ʱ��Ͱѷ��������������� 
}

/* RTC init function */
void MX_RTC_Init(uint32_t timeStamp)
{
//  RTC_TimeTypeDef sTime;
//  RTC_DateTypeDef DateToUpdate;

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
	if(HAL_RTC_SetTimeUser(&hrtc,timeStamp) != HAL_OK){
		_Error_Handler(__FILE__, __LINE__);
	}
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */
//	if(HAL_RTCEx_SetSecond_IT(&hrtc) !=HAL_OK){
//		_Error_Handler(__FILE__, __LINE__);
//	}
    /**Initialize RTC and set the Time and Date 
    */
//  sTime.Hours =14;
//  sTime.Minutes =5;
//  sTime.Seconds =25;

//  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
  /* USER CODE BEGIN RTC_Init 3 */

  /* USER CODE END RTC_Init 3 */

//  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
//  DateToUpdate.Month = RTC_MONTH_JANUARY;
//  DateToUpdate.Date = 20;
//  DateToUpdate.Year = 18;

//  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
  /* USER CODE BEGIN RTC_Init 4 */
	if(HAL_RTC_SetAlarm_IT(&hrtc,timeStamp+5) != HAL_OK){
		_Error_Handler(__FILE__, __LINE__);
	}
  /* USER CODE END RTC_Init 4 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */
//		HAL_NVIC_SetPriority(RTC_IRQn, 10, 0);
//    HAL_NVIC_EnableIRQ(RTC_IRQn);
    
  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */
		HAL_NVIC_DisableIRQ(RTC_IRQn);
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
  /* USER CODE END RTC_MspDeInit 1 */
  }
} 

/**
  * @brief  HAL_RTC_ToUTC  ��ʱ��ת��Ϊ ʱ���
  * @param 
  * @retval ���ص�ʱ���
  */
uint32_t	RTC_ToUTC(ST_Time* time)//(RTC_HandleTypeDef* hrtc)
{
	uint32_t  seconds = 0U;
	uint16_t  Year = 0U;
	
  Year = time->date.Year-1970;        //������2100��ǧ�������
  seconds = xYEAR*Year + xDAY*((Year+1)/4);  //ǰ�����ȥ������
  seconds += Month[time->date.Month-1]; //���Ͻ��걾�¹�ȥ������
  if( (time->date.Month > 2) && (((Year+2)%4)==0) )//2008��Ϊ����
    seconds += xDAY;            //�����1������
  seconds += xDAY*(time->date.Date-1); //���ϱ����ȥ������
  seconds += xHOUR*time->Time.Hours;     //���ϱ�Сʱ��ȥ������
  seconds += xMINUTE*time->Time.Minutes;  //���ϱ����ӹ�ȥ������
  seconds += time->Time.Seconds;          //���ϵ�ǰ����<br>��
//	seconds -= 8 * xHOUR; //���ﱱ��ʱ���UTC��8��Сʱ  ���Ӱ�����ʱ�������õ���ʱ��������
  return seconds;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
//RTC���жϻص�����
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
}
//RTC�����ӻص����� 
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
//	static u8 check_time;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//	if((em_gWifiState == dns_founded) || (em_gWifiState == login_ok)){ //�ҵ�DNS���ߵ�½���������������ѽ��뻥����
//		check_time=0;
//		
//		ul_gUnixTimeStramp = RTC_ReadTimeCounter(hrtc);
//		
//		if( ul_gUnixTimeStramp < COMPILE_TIME ){
//			HAL_RTC_SetAlarm_IT(hrtc , ul_gUnixTimeStramp + 20); //�п��ܻ��ȡ����ʱ�� �������û�л�ȡ��ʱ���ÿ20���ȡһ��
//		}else{
//			HAL_RTC_SetAlarm_IT(hrtc , ul_gUnixTimeStramp + ALARM_CYCLE); //ÿ�λ��Ѷ�����5���ӵ����ӣ�Ϊ�˷�ֹGet_timeʧ��
//		}
//		
//		sprintf((char *)uc_gWifiSendBufferArr,"Get_time");
//		
//		xTaskNotifyFromISR(SendWifiTaskHandle,(u32 )sizeof("Get_time"),eSetValueWithOverwrite,&xHigherPriorityTaskWoken);
//	}else if(em_gWifiState == connect_router){
//		check_time=0;
//	}else{
//		check_time++; 
//		if((check_time % 3) == 0){//���һ���Ӷ�û������������WIFIģ��
//		 xTaskNotifyFromISR(KeyTaskHandle,SHORT_PRESS_BIT,eSetBits,&xHigherPriorityTaskWoken); 
//		}else{
//			ul_gUnixTimeStramp = RTC_ReadTimeCounter(hrtc);
//			HAL_RTC_SetAlarm_IT(hrtc , ul_gUnixTimeStramp + 20); //���û�����ӵ�·��20���Ӳ�ѯһ��
//			sprintf((char *)uc_gWifiSendBufferArr,"Check_DNS");
//			xTaskNotifyFromISR(SendWifiTaskHandle,(u32 )sizeof("Check_DNS"),eSetValueWithOverwrite,&xHigherPriorityTaskWoken);
//		}
//	}
		vTaskNotifyGiveFromISR(TimeTaskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//�ж��Ƿ��и����ȼ���������Ҫ�л�
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
