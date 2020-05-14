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
//  全局时间戳变量 此变量临时存从RTC取出的或者是从网络取出的
uint32_t	ul_gUnixTimeStramp = 0;

#define xMINUTE   (60) //1分的秒数
#define xHOUR     (60*xMINUTE) //1小时的秒数
#define xDAY			86400U //1天的秒数
#define xYEAR     31536000 //1年的秒数

extern volatile EM_ConState em_gWifiState;
extern TaskHandle_t  SendWifiTaskHandle;
extern TaskHandle_t	KeyTaskHandle;
extern TaskHandle_t	TimeTaskHandle;

extern u8 uc_gTimTaskList2Arr[TIMINGTASK_LIST_LEN_MAX][TIMINGTASK_HEAD_LEN_MAX];
static uint32_t Month[12]={
    xDAY*(0),//01月
    xDAY*(31),//02月
    xDAY*(31+28),//03月
    xDAY*(31+28+31),//04月
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
* @brief  CalcMinTimstamp 计算出任务当中最近需要执行的任务的时间戳 并且最近的任务在5分钟之内就设这个为闹钟
* @param 
* @retval None
*/
void CalcMinTimstamp(void)
{
	u8 uc_cycleID;
	//读出现在的时间戳
	ul_gUnixTimeStramp=RTC_ReadTimeCounter(&hrtc);
	
	RECENT_TASK_UTC = ul_gTimeTaskUnixTimArr[0];
	//循环找出最近所需要执行的值
	for(uc_cycleID = 0; uc_cycleID < TIMINGTASK_LIST_LEN_MAX; uc_cycleID++){
		//如果任务中的时间戳小于现在的时间 (开机的时候就不会计算出比现在时间小的任务) 那么这个任务就不应该被执行（周期任务不会小于现在）
		if(ul_gTimeTaskUnixTimArr[uc_cycleID] < ul_gUnixTimeStramp){
			
			ul_gTimeTaskUnixTimArr[uc_cycleID] = 0;
			if(uc_cycleID == 0){
				RECENT_TASK_UTC = 0; //默认最小的是第1个，如果第一个任务不能执行那么把 RECENT_TASK_UTC 一起清除
			}
		}
		
		if((RECENT_TASK_UTC == 0)	&& (ul_gTimeTaskUnixTimArr[uc_cycleID] != 0)){
			RECENT_TASK_UTC = ul_gTimeTaskUnixTimArr[uc_cycleID];
		}
		
		//如果定时任务的 时间戳不为空且 比最近的任务时间还要小就把这个设为最近的
		if((ul_gTimeTaskUnixTimArr[uc_cycleID]!=0) && (ul_gTimeTaskUnixTimArr[uc_cycleID]<RECENT_TASK_UTC)){
			RECENT_TASK_UTC = ul_gTimeTaskUnixTimArr[uc_cycleID];
		}
	}
	//如果最近的任务与现在的时间之差小于5分钟就设这个最近的任务时间戳为闹钟
// if((RECENT_TASK_UTC - ul_gUnixTimeStramp) < ALARM_CYCLE){
		HAL_RTC_SetAlarm_IT(&hrtc,RECENT_TASK_UTC);
//	}
}	
/**
  * @brief  CalcSpecificPeriodicTask 计算指定周期任务执行的时间戳
	*					hrtc 结构体中的时间 应该在上层取出此函数会被CalcAllTask循环调用 不应该循环获取 减少CPU使用
  * @param 	id	：需要计算的ID
	* @param	tody：可以取如下值
	*								@arg	TRUE： 如果当天需要执行以当天的定时来执行
	*								@arg	FALSE: 如果当天需要执行不计算当天的执行时间
  * @retval None
  */
void CalcSpecificPeriodicTask(u8 id,u8 today)
{
	u16 us_lHeadOrStrLen=0;
	u8 uc_period=0;	u8 uc_week=0; u8 uc_dayCycle;
	//取出名称的string len
	memcpy(&us_lHeadOrStrLen,&uc_gTimTaskList2Arr[id][2],2);  
	//频次在名称的后面 偏移名称的长度加上一个头部short和string len 的short=+4;
	uc_period = uc_gTimTaskList2Arr[id][us_lHeadOrStrLen + 4]; 
	
//	if(uc_period & 0x80){ //如果最高位不等于 0 那就是一个周期的任务
		//从任务表中取出任务的时间
		Extract_Time(&st_gRTCTime,&uc_gTimTaskList2Arr[id][us_lHeadOrStrLen + 5]); //移动到频次后面，时间 日期
		//查看今天的执行时间是否已过
		if((st_gRTCTime.Time.Hours < hrtc.Time.Hours)	||
		((st_gRTCTime.Time.Hours == hrtc.Time.Hours) && (st_gRTCTime.Time.Minutes < hrtc.Time.Minutes))){
			today = FALSE;
		}
		uc_period &= (~0x80);  //低7位代表星期1到星期日的每一天是否执行
		uc_week = hrtc.DateToUpdate.WeekDay;
		//循环7天查看哪天需要执行
		for(uc_dayCycle = 0;uc_dayCycle < 8;uc_dayCycle++)
		{
			if(uc_week > 7) uc_week = 1; //如果超过了星期天就直接赋值到星期一
			if(uc_period & (1<<(uc_week-1))){ //如果对应的这天执行 //这里WeekDay-1是由于前7位表示星期几星期1就不用左移了
				//从任务表中取出任务的时间
//					Extract_Time(&st_gRTCTime,&uc_gTimTaskList2Arr[id][us_lHeadOrStrLen + 5]); //移动到频次后面，时间 日期
				if((uc_dayCycle == 0) && (today == FALSE)){ //如果今天需要执行且不需要计算今天 就跳过
					uc_week++; //星期加一天 看看是不是明天需要执行
					continue;
				}
				//把任务的日期用今天的日期更换
				st_gRTCTime.date.Date		=	hrtc.DateToUpdate.Date + uc_dayCycle; //执行时间与今天相差的天数加上去
				st_gRTCTime.date.Month	=	hrtc.DateToUpdate.Month;
				st_gRTCTime.date.Year		=	hrtc.DateToUpdate.Year;
				
//				if(today == FALSE){
//					st_gRTCTime.date.Date--; //减去当天（不计算当天）
//				}
				//计算出对应的时间戳
				ul_gTimeTaskUnixTimArr[id]=RTC_ToUTC(&st_gRTCTime);
				break;
			}
			uc_week++; //星期加一天 看看是不是明天需要执行
		}
//	}   	
}
/**
  * @brief  CalcSpecificOrdinaryTask  计算指定的普通任务（非周期性执行任务）
	*					ul_gUnixTimeStramp 应该在上层取出此函数会被CalcAllTask循环调用 不应该循环获取ul_gUnixTimeStramp 减少CPU使用
  * @param 
  * @retval None
  */
void CalcSpecificNormalTask(u8 id )
{
//	u32 ul_timestamp = 0;
	u16 us_strLen = 0;
//	u8 uc_period=0;
	memcpy(&us_strLen,&uc_gTimTaskList2Arr[id][2],2);  //取出名称的string len
//	uc_period = uc_gTimTaskList2Arr[id][us_strLen + 4]; //频次在名称的后面 偏移名称的长度加上一个头部short和string len 的short=+4;
//	
//	if((uc_period & 0x80) == 0){ //如果不是循环任务
		//从任务表中取出任务的时间
		Extract_Time(&st_gRTCTime,&uc_gTimTaskList2Arr[id][us_strLen + 5]); //移动到频次后面，时间 日期
		//算出对应的时间戳
		ul_gTimeTaskUnixTimArr[id]=RTC_ToUTC(&st_gRTCTime);
		
		//判断本任务还有没有机会执行
		if(ul_gTimeTaskUnixTimArr[id] < ul_gUnixTimeStramp) {
			if((ul_gUnixTimeStramp - ul_gTimeTaskUnixTimArr[id]) < 30){ //如果任务刚过了就执行任务
				/*执行任务*/
				ExecuteTask(id);
				memset(&uc_gTimTaskList2Arr[id][0],0,TIMINGTASK_HEAD_LEN_MAX);
				us_strLen = 0;   //
				STMFLASH_Erase_Word(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * id)); //把总长度清零
			}
			ul_gTimeTaskUnixTimArr[id] = 0; //如果没有执行就不删除任务只清零 任务时间戳
		}
//	}
}

/**
  * @brief  CalcPeriodicTask  计算所有任务下次执行的时间  本函数应该只在获取到网络时间后 只执行一次
	*														在计算的过程中如果有任务已执行不到（时间超过）了就选择执行或者放弃
  * @param 	None
  * @retval None
  */
void CalcAllTask(void)
{
	u8 uc_idCycle=0; //检查是否是周期任务的循环
	u16 us_lHeadOrStrLen=0;
	u8 uc_period=0;
	//取出出RTC时时间戳并转换为时间日期等。放在hrtc结构体中
	HAL_RTC_GetTimeUser(&hrtc);
	//取出RTC时间戳
	ul_gUnixTimeStramp = RTC_ReadTimeCounter(&hrtc);
	for(uc_idCycle = 0;uc_idCycle < TIMINGTASK_LIST_LEN_MAX;uc_idCycle++){
		memcpy(&us_lHeadOrStrLen,&uc_gTimTaskList2Arr[uc_idCycle][0],2); //取出头部长度
		if(uc_gTimTaskList2Arr[uc_idCycle][us_lHeadOrStrLen + 2] == 1){ //先检查任务的状态是否已开启
			/*取出名称的string len*/
			memcpy(&us_lHeadOrStrLen,&uc_gTimTaskList2Arr[uc_idCycle][2],2); 
			//取出执行频次
			uc_period = uc_gTimTaskList2Arr[uc_idCycle][us_lHeadOrStrLen + 4]; //频次在名称的后面 偏移名称的长度加上一个头部short和string len 的short=+4;
			
			if(uc_period & 0x80){
				CalcSpecificPeriodicTask(uc_idCycle,TRUE);
			}else{
				CalcSpecificNormalTask(uc_idCycle);
			}
		}
	}
	CalcMinTimstamp(); //初始化的时候就把非周期任务计算好了 
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
  * @brief  HAL_RTC_ToUTC  把时间转换为 时间戳
  * @param 
  * @retval 返回的时间戳
  */
uint32_t	RTC_ToUTC(ST_Time* time)//(RTC_HandleTypeDef* hrtc)
{
	uint32_t  seconds = 0U;
	uint16_t  Year = 0U;
	
  Year = time->date.Year-1970;        //不考虑2100年千年虫问题
  seconds = xYEAR*Year + xDAY*((Year+1)/4);  //前几年过去的秒数
  seconds += Month[time->date.Month-1]; //加上今年本月过去的秒数
  if( (time->date.Month > 2) && (((Year+2)%4)==0) )//2008年为闰年
    seconds += xDAY;            //闰年加1天秒数
  seconds += xDAY*(time->date.Date-1); //加上本天过去的秒数
  seconds += xHOUR*time->Time.Hours;     //加上本小时过去的秒数
  seconds += xMINUTE*time->Time.Minutes;  //加上本分钟过去的秒数
  seconds += time->Time.Seconds;          //加上当前秒数<br>　
//	seconds -= 8 * xHOUR; //这里北京时间比UTC快8个小时  但从阿里授时服务器得到的时间已修正
  return seconds;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
//RTC的中断回调函数
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
}
//RTC的闹钟回调函数 
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
//	static u8 check_time;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//	if((em_gWifiState == dns_founded) || (em_gWifiState == login_ok)){ //找到DNS或者登陆上了主机都表明已接入互联网
//		check_time=0;
//		
//		ul_gUnixTimeStramp = RTC_ReadTimeCounter(hrtc);
//		
//		if( ul_gUnixTimeStramp < COMPILE_TIME ){
//			HAL_RTC_SetAlarm_IT(hrtc , ul_gUnixTimeStramp + 20); //有可能会获取不到时间 所以如果没有获取到时间就每20秒获取一次
//		}else{
//			HAL_RTC_SetAlarm_IT(hrtc , ul_gUnixTimeStramp + ALARM_CYCLE); //每次唤醒都设置5分钟的闹钟，为了防止Get_time失败
//		}
//		
//		sprintf((char *)uc_gWifiSendBufferArr,"Get_time");
//		
//		xTaskNotifyFromISR(SendWifiTaskHandle,(u32 )sizeof("Get_time"),eSetValueWithOverwrite,&xHigherPriorityTaskWoken);
//	}else if(em_gWifiState == connect_router){
//		check_time=0;
//	}else{
//		check_time++; 
//		if((check_time % 3) == 0){//如果一分钟都没有连接上重启WIFI模块
//		 xTaskNotifyFromISR(KeyTaskHandle,SHORT_PRESS_BIT,eSetBits,&xHigherPriorityTaskWoken); 
//		}else{
//			ul_gUnixTimeStramp = RTC_ReadTimeCounter(hrtc);
//			HAL_RTC_SetAlarm_IT(hrtc , ul_gUnixTimeStramp + 20); //如果没有连接到路由20秒钟查询一次
//			sprintf((char *)uc_gWifiSendBufferArr,"Check_DNS");
//			xTaskNotifyFromISR(SendWifiTaskHandle,(u32 )sizeof("Check_DNS"),eSetValueWithOverwrite,&xHigherPriorityTaskWoken);
//		}
//	}
		vTaskNotifyGiveFromISR(TimeTaskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//判断是否有高优先级的任务需要切换
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
