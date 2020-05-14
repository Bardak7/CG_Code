/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "sys.h"
#include "delay.h"
#include "gpio.h"
#include "usart.h"
#include "string.h"
#include "protocol.h"
#include "dma.h"
#include "rf433comm.h"
#include "radio.h"
#include <stdlib.h>
#include "stmflash.h"
#include "rtc.h"
#include "adc.h"
#include "iwdg.h"
#include "timers.h"
/* USER CODE BEGIN Includes */     

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
//osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
//void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */


vu8  uc_vgTim1CountRetryRF; //重发
vu16 us_vgTim1CountHeart;		//心跳
vu16 us_vgTim1CountTemp;
/* extern variable */
//extern volatile u8	uc_vgTim2CountMode;
extern volatile u8	uc_vgStudyCount;
								u16 us_gAddrToFlashBufArr[3]; //前2个半字做为开关的或分机的地址 后个半字用于开关的通道buffer写入flash


/**WIFI发送队列相关**/
#define 	SEND_QUE_LEN	3
QueueHandle_t Queue_Sendout;  

//SemaphoreHandle_t xSemaphore = NULL;//定义二值信号量句柄

						/**以下结构体用于WIFI格式化读取的数据 const 把指针的内容限定不可更改其指向的内容**/
const ST_WifiDataPackHead		 			* const	stp_gWifiDataPackHead=(ST_WifiDataPackHead *)USART_RX_BUF;  //st表示它是一个结构体变量，p表示他是一个指针类型_g表示他是一个global全局变量
const ST_Long   					*stp_gLongID;
const ST_IntInt						*stp_gIntInt;
const ST_LongCharChar			*stp_gLongCC;
const ST_LongStirng				*stp_gLongStr;
const ST_LongShortString	*stp_gLongShortStr;
const ST_ShortLong				*stp_gShortLong;


volatile EM_ConState em_gWifiState;

/**连接状态记数 如 多次没有收心跳回复  等等**/
#define RETRY_MAX	3
u8 uc_gConStasCount;

u8 uc_gUartSendLock;

/* Hook prototypes */

/* wifi task: process the data from wifi */
void ProcessWifiTask(void *pvParameters);
#define PROCESS_WIFI_DEPTH			256
#define PROCESS_WIFI_PRIO			   16
TaskHandle_t  ProcessWifiTaskHandle;
/*task1 fotTest*/
//void task1_test(void *pvParameters);
//#define task1StackDepth 60
//#define task1Priority   3
//TaskHandle_t  task1Handle;
/**发送数据通过WIFI*/
void SendWifiTask(void *pvParameters);
#define SEND_WIFI_DEPTH 	256
#define SEND_WIFI_PRIO 		 9
TaskHandle_t  SendWifiTaskHandle;
u8 volatile 	n_gWifiSend;
/*发送CMT数据*/
void SendCMTTask(void *pvParameters);
#define SEND_CMT_DEPTH			128
#define SEND_CMT_PRIO				6
TaskHandle_t	SendCMTTaskHandle;
/*接收来自CMT2300的数据*/
void ReceiveCMTTask(void *pvParameters);
#define	RECEIVE_CMT_DEPTH		128
#define RECEIVE_CMT_PRIO		12
TaskHandle_t	ReceiveCMTTaskHandle;
/*处理学习任务*/
void StudyTask(void *pvParameters);
#define STUDY_DEPTH					128
#define STUDY_PRIO					8
TaskHandle_t			StudyTaskHandler;
/**处理定时任务相关**/
void TimedTask(void *pvParameters);
#define TIME_TASK_DEPTH			90
#define TIME_TASK_PRIO			7
TaskHandle_t	TimeTaskHandle;

/**按键任务相关**/
void KeyTask(void *pvParameters);
#define KEY_TASK_DEPTH			50
#define KEY_TASK_PRIO				5
TaskHandle_t	KeyTaskHandle;

/**心跳任务相关（）**/
void HeartBeat(void *pvParameters);
#define HEART_TASK_DEPTH			50
#define HEART_TASK_PRIO				5

//ST_433Packet static volatile const * stp_g433Pack; //只能被本文件使用
/*计数信号量句柄*/
//QueueHandle_t cmtSendQueueHanlde;
/**互斥信号量用于互斥心跳发送**/
//QueueHandle_t  MutexSemaphor;

/*消息队列句柄用于RF发送消息（特别是在定时任务中需要发送多个）*/
QueueHandle_t RFSendQueueHandle;  //用消息队列传送USART数据到任务

/******************************************************************************
 * FunctionName : Wifi_QueueIn
 * Description  : 向wifi发送队列入队，在SendWifiTask出队并通过串口发向ESP 中断中不可调用此函数
 * Parameters   :
 * Returns      : none
*******************************************************************************/

u8 Wifi_QueueIn (ST_Report * st_report)
{
	if( Queue_Sendout !=	NULL ){
		if(xQueueSend(Queue_Sendout, st_report,( TickType_t ) 10 ) != pdPASS){
			return RESULT_FALSE;
		}
	}else{
		return RESULT_FALSE;
	}
	return RESULT_TRUE;
}


void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
    
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */

	
  xTaskCreate( (TaskFunction_t  ) ProcessWifiTask,
								(const char *   ) "ProcessWifiTask",
								(unsigned short ) PROCESS_WIFI_DEPTH,
								(void *         ) NULL,
								(UBaseType_t    ) PROCESS_WIFI_PRIO,
								(TaskHandle_t * ) &ProcessWifiTaskHandle );
								
	xTaskCreate((TaskFunction_t		) SendWifiTask,
								(const char *		) "SendWifiTask",
								(uint16_t      	) SEND_WIFI_DEPTH,
								(void *					)NULL,
								(UBaseType_t		)PROCESS_WIFI_PRIO,
								(TaskHandle_t * ) &SendWifiTaskHandle);
								
//	xTaskCreate( (TaskFunction_t  ) task1_test,
//								(const char *   ) "task1",
//								(unsigned short ) task1StackDepth,
//								(void *         ) NULL,
//								(UBaseType_t    ) task1Priority,
//								(TaskHandle_t * ) &task1Handle );		
	xTaskCreate( (TaskFunction_t  ) SendCMTTask,
								(const char *   ) "SendCMTTask",
								(unsigned short ) SEND_CMT_DEPTH,
								(void *         ) NULL,
								(UBaseType_t    ) SEND_CMT_PRIO,
								(TaskHandle_t * ) &SendCMTTaskHandle );		
								
	xTaskCreate( (TaskFunction_t  ) ReceiveCMTTask,
								(const char *   ) "ReceiveCMTTask",
								(unsigned short ) RECEIVE_CMT_DEPTH,
								(void *         ) NULL,
								(UBaseType_t    ) RECEIVE_CMT_PRIO,
								(TaskHandle_t * ) &ReceiveCMTTaskHandle );
								
	xTaskCreate( (TaskFunction_t  ) StudyTask,
								(const char *   ) "StudyTask",
								(unsigned short ) STUDY_DEPTH,
								(void *         ) NULL,
								(UBaseType_t    ) STUDY_PRIO,
								(TaskHandle_t * ) &StudyTaskHandler );
								
	xTaskCreate( (TaskFunction_t  ) TimedTask,
								(const char *   ) "TimedTask",
								(unsigned short ) TIME_TASK_DEPTH,
								(void *         ) NULL,
								(UBaseType_t    ) TIME_TASK_PRIO,
								(TaskHandle_t * ) &TimeTaskHandle );
								
	xTaskCreate( (TaskFunction_t  ) KeyTask,
								(const char *   ) "TimedTask",
								(unsigned short ) KEY_TASK_DEPTH,
								(void *         ) NULL,
								(UBaseType_t    ) KEY_TASK_PRIO,
								(TaskHandle_t * ) &KeyTaskHandle );
								
	xTaskCreate( (TaskFunction_t  ) HeartBeat,
								(const char *   ) "HeartBeat",
								(unsigned short ) HEART_TASK_DEPTH,
								(void *         ) NULL,
								(UBaseType_t    ) HEART_TASK_PRIO,
								NULL );
	/*创建一个消息队列用于RF的发送*/
	RFSendQueueHandle=xQueueCreate(TIMINGTASK_CMD_MAX,RF433BUFF_MAX);		 //最多有3个条目的控制命令，最多一次只能发送64个字节的数据		
	/*创建一个消息队列用于RF的发送*/
	Queue_Sendout = xQueueCreate(SEND_QUE_LEN,sizeof(ST_Report));
		
	Protocol_Init();
								
	uc_vgTim1CountRetryRF = 50;						
	
  								
	/*创建一个互斥号量*/
//	MutexSemaphor = xSemaphoreCreateBinary();//xSemaphoreCreateMutex();
		
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
//  for(;;)
//  {
//		vTaskDelay(10);
		vTaskDelete(NULL);
//  }
  /* USER CODE END StartDefaultTask */
}





uint64_t n64_lAddrBuffer=0;

void ProcessWifiTask (void *pvParameters)
{
	

	u8 timeID=0; u8 uc_fristGetTime=0;
	u32 ul_lNotifyValue=0;
	u32 ul_addr = 0;

	
  while(1)
	{
		ul_lNotifyValue = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		if(ul_lNotifyValue==1){
			MCU_LED_EN;
//			printf("ok");
//			us_vgTim1CountHeart
/**检测包头正确与否**/
			if(stp_gWifiDataPackHead->head == PKT_HDR){
				
				//开互斥锁 不得被其它任务更改 uc_gWifiSendBufferArr 内容
//			  xSemaphoreTake(MutexSemaphor,10);stp_gWifiDataHeadToSend,
				InitWifiSendHeader(stp_gWifiDataPackHead); //把接收到的包头复制到发送的缓存的包头里去
				if(stp_gWifiDataPackHead->cmd == LOGIN_OK){
					MCU_LED_DIS;
					em_gWifiState = login_ok;
//					us_vgTim1CountHeart = 5; //5*500MS过后心跳
					if(st_gHostInfo.lockInfo == 2U){
						HEARBEAT();
					}
					uc_gConStasCount = 0;
					n64_lAddrBuffer = 0U;
				}else if(stp_gWifiDataPackHead->cmd == HEARTBEAT){
					MCU_LED_DIS;
					n64_lAddrBuffer = 0U;
					uc_gConStasCount = 0;
//					us_vgTim1CountHeart = 46; //46*500MS过后没有收到就标记
				}else{
	//把设备ID格式化出来
					stp_gLongID=(ST_Long*)(&stp_gWifiDataPackHead->next);
					/**计算出对应的设备ID**/
					n64_lAddrBuffer=stp_gLongID->hostID1;
					n64_lAddrBuffer<<=32;
					n64_lAddrBuffer|=stp_gLongID->hostID2;  //计算出对应的设备ID
				}
				/*******************/
				if(st_gHostInfo.deviceID==n64_lAddrBuffer){ //如果发送过来的ID和我们的ID符合
					
					n64_lAddrBuffer = 0;
					
					switch(stp_gWifiDataPackHead->cmd){
	/**广播命令************/
						case BROADCAST :
							//InitWifiSendHeader(stp_gWifiDataHeadToSend,stp_gWifiDataPackHead);
							BROADCAST_FUN((u32)(st_gHostInfo.deviceID>>32),(u32)st_gHostInfo.deviceID);//0x50578248,0x066BFF54);//
	//						WifiIntLongCCReport(&stp_gWifiDataHeadToSend->next,0x50578248,0x03,0x07);
						break;
	/**添加分机命令************/					
						case ADD_SLAVE :
							/*Add slave command only has one address,no need to format others data*/
							ADD_SLAVE_FUN();
							
							break;
	/**学习命令************/					
						case STUDY :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							
							STUDY_FUN(stp_gLongCC->studyModeOrChannel);
							break;
	/**停止学习命令************/	
						case STOP_STUDY :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							STOP_FUN(RESULT_TRUE);
							break;
	/**控制命令************/	
						case CONTROL :
							stp_gLongStr=(ST_LongStirng*)(&stp_gLongID->next);
	//						stp_gLongStr->slaveIDL
							n64_lAddrBuffer=stp_gLongStr->slaveIDH;
							n64_lAddrBuffer<<=32;
							n64_lAddrBuffer|=stp_gLongStr->slaveIDL;  //计算出对应的设备ID
							if(n64_lAddrBuffer==st_gHostInfo.deviceID){ //如果是发给当前设备的，直接解码处理
								
								CONTROL_FUN(&stp_gLongStr->content,stp_gLongStr->stringLen);
							}else{
								CONTROL_SLAVE_FUN(stp_gLongStr->slaveIDL,stp_gLongStr->stringLen,&stp_gLongStr->content);
							}
							break;
	/**解锁锁定命令************/	
						case LOCK :
							LOCK_FUN();
							break;
	/**通过客户端升级命令************/	
						case UPDATE_BY_CLIENT :
							stp_gLongShortStr=(ST_LongShortString*)(&stp_gLongID->next);
							UPDATA_FUN();
							break;
	/**查询设备状态命令************/	
						case QUERY_DEVICE_STATE :
							QUERY_FUN();
							break;
	/**批量查询设备状态命令************/	
						case BATCH_QUERY :
							//批量查询 应该下发 数组长度 和 设备ID数组 （数组中包含64位设备地址） 
//							stp_gShortLong=(ST_ShortLong*)(&stp_gLongID->next);
							BATCH_QUERY_FUN();
							break;
	/**添加定时任务命令************/	
						case ADD_TIMED_TASK:
							
							timeID=ADD_TIME_LIST_FUN(&stp_gLongID->next);
							if(timeID==0){		//失败会返回0
								ERROR_REPORT();
							}else{
								ADD_TIME_LIST_REPORT(timeID&0x7F);//返回的定时任务的任务ID

							}
							break;
	/**删除定时任务命令************/	
						case DELETE_TIMED_TASK:
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
						
							if(RESULT_FALSE!=DELETE_TIMED_TASK_FUN(&stp_gIntInt->intData2.state,(u8)stp_gIntInt->intData1.taskID)){ //如果不是失败就直接返回任务ID和返回值
								SUCCESS_REPORT();
							}else{
								ERROR_REPORT();
							}
							break;
	/**更改定时任务命令************/	
						case MODIFY_TIMED_TASK:
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							if(RESULT_FALSE!=MODIFY_TIMED_TASK_FUN(&stp_gIntInt->intData2.state,stp_gIntInt->intData1.taskID)){
								SUCCESS_REPORT();
							}else{
								ERROR_REPORT();
							}
							break;
	/**获取定时任务列表************/	
						case GET_TIMED_TASK_LIST :
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							GET_TIMED_TASK_LIST_FUN(stp_gIntInt->intData1.startPos,stp_gIntInt->intData2.amount);
							break;
	/**获取单个定时任务************/	
						case GET_ONE_TIMED_TASK :
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							GET_ONE_TIMED_TASK_FUN(stp_gIntInt->intData1.taskID);
							break;
	/**更改定时任务状态命令************/	
						case MODIFY_TIMED_TASK_STATE :
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							MODIFY_TIMED_TASK_STATE_FUN(stp_gIntInt->intData1.taskID,stp_gIntInt->intData2.state);
						
							break;
	/**添加开关************/	
						case ADD_SWITCH :
							ADD_SWITCH_FUN();
						
							break;
	/**查询开关命令************/	
						case QUERY_SWTICH_STATE :
							//现在每次只查询一个开关所以不管  数组长度  应该常为1
							stp_gShortLong = (ST_ShortLong *)(&stp_gLongID->next);
							ul_addr = 	stp_gShortLong->usIDLH;
							ul_addr	<<= 16;
 							ul_addr	|=	stp_gShortLong->usIDLL;
							QUERY_SWTICH_STATE_FUN(ul_addr);
							break;
	/**控制开关命令************/	
						case CONTROL_SWITCH :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							CONTROL_SWITCH_FUN(stp_gLongCC->slaveIDL,&stp_gLongCC->studyModeOrChannel);
							break;
	/**获取升级进度命令************/	
						case GET_UPDATE_PROGRESS :
							GET_UPDATE_PROGRESS_FUN();
							break;
	/**删除分机命令************/	
						case DELETE_SLAVE :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							DELETE_SLAVE_FUN(stp_gLongCC->slaveIDL);
							break;
	/**删除开关命令************/	
						case DELETE_SWITCH :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							DELETE_SWITCH_FUN(stp_gLongCC->slaveIDL);
							break;
					}
				}else if(n64_lAddrBuffer == 0U){//(n64_lAddrBuffer == 0U){  //如果发送过来的ID是0的话有可能是第一次添加主机，这个时候如果已锁定设备不应当回复
					if(stp_gWifiDataPackHead->cmd == BROADCAST){
						if(st_gHostInfo.lockInfo != 2U){ //如果未添加标记已添加 并返回广播
							st_gHostInfo.lockInfo = 2U;
							STMFLASH_Write(LOCK_FLAG_FLASH_ADDR,&st_gHostInfo.lockInfo,1);
							InitWifiSendHeader(stp_gWifiDataPackHead);
							BROADCAST_FUN((u32)(st_gHostInfo.deviceID>>32),(u32)st_gHostInfo.deviceID);
						}else{  //如果已添加什么都不做
							;
						}
					}
//					else if(stp_gWifiDataPackHead->cmd == HEARTBEAT){ //心跳回复
//						;
//					}else if(stp_gWifiDataPackHead->cmd == LOGIN_OK){ //登陆回复
//						;
//					}
				}else{  //如果不是本机地址且不是 为空的地址 就不是本地址不应当返回
					MCU_LED_DIS;
					n64_lAddrBuffer = 0;
//					ERROR_REPORT(&stp_gWifiDataHeadToSend->next);					
				}
				//互斥解锁 只要包头对就有可能需要返回wifi数据就开互斥锁 然后再到这里关互斥锁
//				xSemaphoreGive(MutexSemaphor);
			}else{
/**如果包头不是数据包可能就是一些连接状态等**/
				if(strstr((const char *)USART_RX_BUF,"DNS has been founded") != NULL){
					em_gWifiState=dns_founded;  //如果找到了DNS那么一定是已经联网了也获取到了网络时间了 
//					ul_gUnixTimeStramp=1540630546;
//					MX_RTC_Init(ul_gUnixTimeStramp);
//					DeviceLogin(1,NULL); //如果已找到相应的DNS就可以登陆了
//					HAL_RTC_GetTimeUser(&hrtc);
//					us_vgTim1CountHeart=20; //20*500MS后心跳 或者重新登陆
		//				 Send_wifi(); 
					if( st_gHostInfo.lockInfo == 2U ){
						LOGIN_NORM();
					}
					uc_gConStasCount = 0;
				 }else if(strstr((const char *)USART_RX_BUF,"Connected to the router") != NULL){
					 em_gWifiState 		= connect_router;
					 uc_gConStasCount = 0;
				 }else if(strstr((const char *)USART_RX_BUF,"Disconnect") != NULL){
					 em_gWifiState		=	disconnect;
					 uc_gConStasCount = 0;
//					 us_vgTim1CountHeart = HEART_TIM; //如果1分钟后还是没有连网就重启一下WIFI
					 }else if(strstr((const char *)USART_RX_BUF,"time:") != NULL){
		//				 UnixTime=(u32)()
						 //ul_gUnixTimeStramp = 555555;
//					 memcpy(&ul_gUnixTimeStramp,USART_RX_BUF+strlen("time:"),4);//sizeof("time:")-1 是为了减去一个结束符
					 ul_gUnixTimeStramp = atoi( (const char*)(USART_RX_BUF + strlen("time:")));
					 // 设入时间戳
					 if(ul_gUnixTimeStramp > COMPILE_TIME){ //时间肯定是会大于编译时间的 时间戳只上涨不下降
						 HAL_RTC_SetTimeUser(&hrtc,ul_gUnixTimeStramp); //校正网络时间
						 if(uc_fristGetTime == 0){
								uc_fristGetTime = 1;
								CalcAllTask();   // 只在第一次获取任务的时候才把所有的任务的时间戳算一遍
						 }
						 //ul_gUnixTimeStramp = RTC_ReadTimeCounter(&hrtc);
						 xTaskNotifyGive(TimeTaskHandle);
					 }
					 HAL_RTC_SetAlarm_IT(&hrtc , ul_gUnixTimeStramp + ALARM_CYCLE);
				 }else{//如果包头不正确并且非连接或者时间直接返回错误
//						ERROR_REPORT(&stp_gWifiDataHeadToSend->next);
//					 memset(USART_RX_BUF,0,100);
					 MCU_LED_DIS;
				 }
				MCU_LED_DIS;				 
			}
//			portENTER_CRITICAL();
//			memset(USART_RX_BUF,0,USART_REC_LEN); //处理后清空usart接收缓存
//			portEXIT_CRITICAL();
		}
		taskYIELD();
	}

}
/**
  *SendWifiTask
	*接收任务通知获取到需要发送的数量时通过DMA发送到USART1 没有获取到任务通知的时候进入阻滞态
  */
u16 testContent;
void SendWifiTask(void *pvParameters)
{
	ST_WifiDataPackHead		*	stp_gWifiDataHeadToSend ;
	ST_Report *stp_Report = NULL;
	stp_Report = pvPortMalloc(sizeof(ST_Report));
	while(1){
//		NotifyTakeValue=ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		if( ( Queue_Sendout != NULL ) && ( stp_Report != NULL )){
			if(xQueueReceive(Queue_Sendout,stp_Report,portMAX_DELAY)== pdTRUE){
				
				while(uc_gUartSendLock){
					vTaskDelay(100);
				}
				
				stp_gWifiDataHeadToSend = (ST_WifiDataPackHead	*)stp_Report->reportBuf;
				if( stp_Report->reportLen < WIFI_SEND_BUF_MAX){
					MCU_LED_DIS;
					/**当发送数据包时计算出长度域**/
					if(stp_gWifiDataHeadToSend->head == PKT_HDR){
						stp_gWifiDataHeadToSend->len = (u16) ( stp_Report->reportLen - 22);
					}

					portENTER_CRITICAL();
					if(HAL_UART_Transmit(&UART1_Handler, (uint8_t *)stp_Report->reportBuf, stp_Report->reportLen, 8000) == HAL_TIMEOUT){
						testContent = 0x5AA5;
					}else
						testContent = 0;
					portEXIT_CRITICAL();
					memset( stp_Report->reportBuf, 0, stp_Report->reportLen );
					stp_Report->reportLen = 0; 
					vTaskDelay(100);
					
					
					//portEXIT_CRITICAL();
//			while(__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_TC) != SET){		//等待发送结束
//				vTaskDelay(20);
//			}
////			portENTER_CRITICAL();
//			HAL_UART_Transmit_DMA(&UART1_Handler, (uint8_t *)stp_Report->reportBuf, stp_Report->reportLen);//启动传输
//////			portEXIT_CRITICAL();
////			//使能串口1的DMA发送 //等待DMA传输完成，此时我们来做另外一些事，点灯
////		    //实际应用中，传输数据期间，可以执行另外的任务
//		    while(1)
//		    {
//					if(__HAL_DMA_GET_FLAG( &UART1TxDMA_Handler, DMA_FLAG_TC4 ) )//等待DMA1通道4传输完成
//					{
//							__HAL_DMA_CLEAR_FLAG( &UART1TxDMA_Handler, DMA_FLAG_TC4) ;//清除DMA1通道4传输完成标志
//							HAL_UART_DMAStop(&UART1_Handler);      //传输完成以后关闭串口DMA
//							break; 
//					}else{
////						taskYIELD();
//						vTaskDelay(100);
//					}
//				}
//					memset( stp_Report->reportBuf, 0, stp_Report->reportLen );
//					stp_Report->reportLen = 0; 
					}

			}
		}
		taskYIELD();
	}
//	vPortFree(stp_Report);
//	vTaskDelete(NULL);
}
/*
//void PeriodTimerCallback( TimerHandle_t pxTimer )
//{
////  printf("Period Timer is runing!\r\n"); //在这个回调函数中不可阻滞任务的API
//	
//	if(uc_vgTim2CountMode == 1){ //如果处于学习状态
//		uc_vgStudyCount++;
//		if(uc_vgStudyCount>STUDY_TIME_MAX){ //如果到了学习的最大时间那么学习失败
//			//关闭学习用的定时器2 50us用于学习采样
//			__HAL_TIM_DISABLE(&htim2);
//			//解挂学习任务，发送失败状态位 
//		}
//	}
//}*/
/**
  * @brief  SendCMTTaskTask 获取消息队列数据 用static变量保存要发送的ID如果ID与上一次相同 发送计数加一
  * @param  None
  * @retval None
  */
static u8 uc_sgRFSendCount; //发送计数
static u8 uc_sgSendID; //上一次发送的ID 限本文件作用
static u8 uc_gRXBuffer[RF433BUFF_MAX];
//static	
void SendCMTTask(void *pvParameters)
{
	const ST_433Packet  * stp_lSend433Pack=NULL;
	u8 i=0;
	volatile u32 ul_lNotifyValue = 0;
	
	/* */
	while(1)
	{
		if(RFSendQueueHandle != NULL){ //如果RF发送的消息队列创建成功
			if(xQueuePeek(RFSendQueueHandle,uc_gRXBuffer,portMAX_DELAY)){ //消息队列到了CMT需要发送的数据 否则阻滞
				stp_lSend433Pack=(ST_433Packet *)uc_gRXBuffer;    //格式化
				
				if( uc_sgSendID != stp_lSend433Pack->Id ){ //如果不等于当前ID的话说明上一个已发送完成或者失败这次是消息队列中的新的数据
					uc_sgSendID = stp_lSend433Pack->Id;
					if( stp_lSend433Pack->Id == st_gHostInfo.deviceID ){ //定时任务可能有几条命令是需要从主机有几条是要从分机发出去
						
					}else{
						uc_sgRFSendCount++;
						/***此部分代码计算CMT需要发送的次数如果是分机就发送40次如果是开关就发送20次***/
						Cmt2300_GoStby();
						Cmt2300_SetSynWord(stp_lSend433Pack->TargetADDR);
						for( i = 0; i < SLAVE_LIST_MAX; i++){
							if(stp_lSend433Pack->TargetADDR == st_gHostInfo.stSlave[i].slaveID){
								Cmt2300_SetResendTime(40);
								break;
							}
						}
						if( i >= SLAVE_LIST_MAX){
							Cmt2300_SetResendTime(20); 
						}
						Cmt2300_GoSleep();
						/***************************************************************************/
						//直接发送
						RF_Send(uc_gRXBuffer,stp_lSend433Pack->Len+12);
						uc_vgTim1CountRetryRF=(rand()%5)+1;  //取1到4的随机值 (1~4)*120=(120~480)ms;的得发时间
					}
				}else{  //如果是第二次发送
					if(uc_sgRFSendCount>RF433SEND_MAX){ //如果大于最大发送次数度
						MCU_LED_DIS;
						SET_DEFAULT_SYNC();
						uc_sgRFSendCount=0; 							//清零发送计数
						xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //清除消息队列
					}else{
						//等待任务通知允许发送  这里可取一个随机数由 定时器1发通知
						ul_lNotifyValue=ulTaskNotifyTake(pdTRUE,0);
						if(ul_lNotifyValue==1){
							ul_lNotifyValue = 0;
							RF_Send(uc_gRXBuffer,stp_lSend433Pack->Len+12);
							uc_sgRFSendCount++; //记录已发了一次 //unsigned char static global的变量
							uc_vgTim1CountRetryRF=(rand()%5)+1;  //取1到4的随机值 (1~4)*120=(120~480)ms;的重发时间							
						}	
					}
				}
			}
		}

		taskYIELD();
	}

}
/**
  * @brief  接收CMT2300的任务   接收过后需要发送   两种情况：1，接收到了数据。2，接收到了发送成功的消息
  * @param 
  * @retval None
  */
static u8				uc_sgRFReadBuffer[RF433BUFF_MAX];
void ReceiveCMTTask(void *pvParameters)
{
	uint32_t	notifyValue = 0;

	ST_433Packet 	const * stp_lRead433Pack=NULL;
	ST_SwitchState const *	stp_lSwitchInfo;
	u8 i=0;
	while(1)
	{
//		printf("Receive\r\n");
		
		//开互斥锁 不得被其它任务更改 uc_gWifiSendBufferArr 内容
//			xSemaphoreTake(MutexSemaphor,portMAX_DELAY);
		
		if(xTaskNotifyWait(0,0xffffffffUL,&notifyValue,portMAX_DELAY) == pdPASS){ //在收到CMT2300A的数据或发送成功前进入阻滞态
			if(notifyValue==CMT_RECEIVEBIT){
				RF_Read(uc_sgRFReadBuffer,RF433BUFF_MAX);
				//把RF接收到的数据格式化
				stp_lRead433Pack=(ST_433Packet *)uc_sgRFReadBuffer;
				
				if(stp_lRead433Pack->TargetADDR==((u32)st_gHostInfo.deviceID)){  //目标地址是不是本机地址 可不判断 由synword已过滤
					switch(stp_lRead433Pack->Cmd-0x80)
					{
	/*******开关由客户端控制后上报********/
						case RF_SWITCH_CONTROL :
							//强制转换取出开关的状态和路数
							stp_lSwitchInfo=(ST_SwitchState *)(&stp_lRead433Pack->Data); 
						
							for(i=0;i<SWITCH_LIST_MAX;i++){
								if(st_gHostInfo.stSwitch[i].switchID== stp_lRead433Pack->SoucreAddr){ //如果源地址是某个开关的话
									st_gHostInfo.stSwitch[i].state	=stp_lSwitchInfo->switchState;
									st_gHostInfo.stSwitch[i].channle=stp_lSwitchInfo->switchChannel;
									break;
								}
							}
							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM_SWITCH); //确认的时候直接CMT2300A发送一次
							/********以下代码通过WIFI返回给客户端*********/
							uc_sgRFSendCount=0; 							//清零发送计数
							uc_vgTim1CountRetryRF=0;					//不需要再重发
							xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //清除消息队列
							CONTROL_SWITCH_REPORT(stp_lSwitchInfo->switchState);
							/****************************************/
							break;
	/********开关主动上报***不需要返回给客户端******/
						case RF_SWITCH_REPORT :
							stp_lSwitchInfo=(ST_SwitchState *)(&stp_lRead433Pack->Data); 
							 
							for(i=0;i<SWITCH_LIST_MAX;i++){
								if(st_gHostInfo.stSwitch[i].switchID== stp_lRead433Pack->SoucreAddr){ //如果源地址是某个开关的话
									st_gHostInfo.stSwitch[i].state	=stp_lSwitchInfo->switchState;      //更改状态
									//st_gHostInfo.stSwitch[i].channle=stp_lSwitchInfo->switchChannel;
									break;
								}
							}
							if(i>=SWITCH_LIST_MAX){
								 //如果没有这个开关就给一个删除这个开关的命令 通过发送任务
							}else{
								//这里为什么是TargetADDR 在CMT发送的时候是以这个为sync word的
								Confirm(stp_lRead433Pack->TargetADDR,RF_CONFIRM); //确认的时候直接CMT2300A发送一次
							}
							/**********向客户端返回成功报告************/
							//开关主动上报不需要向客户端返回
	//						WifiIntReport(&stp_gWifiDataHeadToSend->next,1); //
	//						len=findLen((u8*)uc_gWifiSendBufferArr);
	//						if(len>10){
	//							xTaskNotify(SendWifiTaskHandle,(u32 )len,eSetValueWithOverwrite); //Wifi发送						
	//						}
							/*************************/
							break;
	/********添加分机后的回馈*********/
						case RF_ADD_SLAVE:
							for(i=0;i<SLAVE_LIST_MAX;i++){
								if(stp_lRead433Pack->SoucreAddr == st_gHostInfo.stSlave[i].slaveID){
									break;
								}
							}if(i>=SLAVE_LIST_MAX){
							  for(i=0;i<SLAVE_LIST_MAX;i++){
									if(st_gHostInfo.stSlave[i].slaveID ==0){
										st_gHostInfo.stSlave[i].slaveID = stp_lRead433Pack->SoucreAddr;
										//把ID存到FLASH里去
										us_gAddrToFlashBufArr[0]=(u16)st_gHostInfo.stSlave[i].slaveID; //低存低，高存高
										us_gAddrToFlashBufArr[1]=(u16)(st_gHostInfo.stSlave[i].slaveID>>16);
										STMFLASH_Write(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),us_gAddrToFlashBufArr,2);
//										STMFLASH_Write(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),(u16 *) &st_gHostInfo.stSlave[i].slaveID,2);
										st_gHostInfo.slaveCount++;
										break;
									}
								}
								if(i>=SLAVE_LIST_MAX){ //如果遍历了所有的分机ID都有的话那么已达到最大分机添加数  可以覆盖或者报错
									//这里选择不报错
									st_gHostInfo.stSlave[SLAVE_LIST_MAX-1].slaveID = stp_lRead433Pack->SoucreAddr; //覆盖最后一个
									STMFLASH_Write(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),
																(u16 *)&st_gHostInfo.stSlave[SLAVE_LIST_MAX-1].slaveID,
																2);
									st_gHostInfo.slaveCount++;
								}
							}
							
							/******已接收到回馈清除上一次的队列和发送次数计数************/
							//Cmt2300_SetSynWord((u32)(st_gHostInfo.deviceID)); //把synWord改为主机地址
//							uc_sgRFSendCount=0; 							//清零发送计数
//							uc_vgTim1CountRetryRF=0;					//不需要再重发
//							xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //清除消息队列
							delay_ms(100);
							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM); //确认的时候直接CMT2300A发送一次
							delay_ms(100);
							SET_DEFAULT_SYNC();
							/**返回给客户端*/
							ADD_SLAVE_REPORT(stp_lRead433Pack->SoucreAddr);
							break;
	/********添加开关后的回馈*********/						
						case RF_ADD_SWITCH :
							stp_lSwitchInfo=(ST_SwitchState *)(&stp_lRead433Pack->Data); 
							for(i=0;i<SWITCH_LIST_MAX;i++){
								if(stp_lRead433Pack->SoucreAddr == st_gHostInfo.stSwitch[i].switchID){ //已添加过就不用管了
									break;
								}
							}
							if(i>=SWITCH_LIST_MAX){ //如果没有添加过再添加到列表和flash中去
								for(i=0;i<SWITCH_LIST_MAX;i++){
									if(st_gHostInfo.stSwitch[i].switchID ==0){
										st_gHostInfo.stSwitch[i].switchID = stp_lRead433Pack->SoucreAddr;
										st_gHostInfo.stSwitch[i].channle	= stp_lSwitchInfo->switchChannel;
										st_gHostInfo.stSwitch[i].state		= stp_lSwitchInfo->switchState;	
										//把ID存到FLASH里去
										us_gAddrToFlashBufArr[0] = (u16)st_gHostInfo.stSwitch[i].switchID; //低存低，高存高
										us_gAddrToFlashBufArr[1] = (u16)(st_gHostInfo.stSwitch[i].switchID>>16);
										us_gAddrToFlashBufArr[2] = (u16)st_gHostInfo.stSwitch[i].channle;
										STMFLASH_Write(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i),us_gAddrToFlashBufArr,3); //写入flash
										st_gHostInfo.switchCount++;
										break;
									}
								}
								if(i>=SWITCH_LIST_MAX){ //如果列表满了直接覆盖最后一个。
									st_gHostInfo.stSwitch[SLAVE_LIST_MAX-1].switchID	= stp_lRead433Pack->SoucreAddr; //覆盖最后一个
									st_gHostInfo.stSwitch[SLAVE_LIST_MAX-1].channle		= stp_lSwitchInfo->switchChannel;
									st_gHostInfo.stSwitch[SLAVE_LIST_MAX-1].state			= stp_lSwitchInfo->switchState;	
									//把ID存到FLASH里去
									us_gAddrToFlashBufArr[0] = (u16)st_gHostInfo.stSwitch[SLAVE_LIST_MAX - 1].switchID; //低存低，高存高
									us_gAddrToFlashBufArr[1] = (u16)(st_gHostInfo.stSwitch[SLAVE_LIST_MAX - 1].switchID>>16);
									us_gAddrToFlashBufArr[2] = (u16)st_gHostInfo.stSwitch[SLAVE_LIST_MAX - 1].channle;
									STMFLASH_Write(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * (SLAVE_LIST_MAX - 1)),us_gAddrToFlashBufArr,3); //写入flash
//									st_gHostInfo.switchCount++;
								}
							}
							/**返回给开关*/
//							uc_sgRFSendCount=0; 							//清零发送计数
//							uc_vgTim1CountRetryRF=0;					//不需要再重发
//							xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //清除消息队列
							delay_ms(100);
							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM); //确认的时候直接CMT2300A发送一次
							delay_ms(100);
//							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM); //确认的时候直接CMT2300A发送一次
							/**返回给客户端*/
							ADD_SWITCH_REPORT(stp_lRead433Pack->SoucreAddr,stp_lSwitchInfo->switchChannel,stp_lSwitchInfo->switchState);
							
							break;
	/********控制分机后的回馈*********/					
						case RF_CONTROL_DEVICE :
							
							CONTROL_REPORT(stp_lRead433Pack->Data); //返回给客户端成功和控制后的状态
							break;
	/********删除分机后的回馈*********/					
						case RF_DEL_SLAVE :
							//在发送RF之前需要检查本地是否有这个分机，如果没有就直接回复成功
							for(i=0;i<10;i++){
								if(st_gHostInfo.stSlave[i].slaveID == stp_lRead433Pack->SoucreAddr){
									st_gHostInfo.stSlave[i].slaveID=0U; //删除分机内容
									
									//把Flash中的分机存储删除
									/**
									*/
									us_gAddrToFlashBufArr[0] = 0; //直接写0可调用STMFLASH_Write_NoCheck 减少CPU占用
									us_gAddrToFlashBufArr[1] = 0;
									STMFLASH_Erase_Word(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i));
									if(st_gHostInfo.slaveCount > 0)
											st_gHostInfo.slaveCount--;
									DELETE_SLAVE_REPORT(RESULT_TRUE);
									break; //退出for
								}
							}
							DELETE_SLAVE_REPORT(RESULT_TRUE); //返回给客户端成功
							break;//退出case
	/********删除开关后的回馈*********/					
						case RF_DEL_SWITCH :
							for(i=0;i<10;i++){
								if(st_gHostInfo.stSwitch[i].switchID == stp_lRead433Pack->SoucreAddr){
									st_gHostInfo.stSwitch[i].switchID	=0;
									st_gHostInfo.stSwitch[i].channle	=0;
									st_gHostInfo.stSwitch[i].state		=0;  //清零开关的数据
									
									//把FLASH中的开关存储删除
									/**
									*/
									us_gAddrToFlashBufArr[0] = 0; //直接写0可调用STMFLASH_Write_NoCheck 减少CPU占用
									us_gAddrToFlashBufArr[1] = 0;
									STMFLASH_Erase_Word(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i)); //写入flash
									st_gHostInfo.switchCount--;
									DELETE_SWITCH_REPORT(RESULT_TRUE);//返回给客户端成功
									break;//退出for
								}
							}
							break; //退出case
					}
					MCU_LED_DIS;
					uc_sgRFSendCount=0; 							//清零发送计数
					uc_vgTim1CountRetryRF=0;					//不需要再重发
					xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //清除消息队列
					SET_DEFAULT_SYNC();
				}
			}else if(notifyValue==CMT_SENDBIT){ //发送成功
				Cmt2300_ClearInterruptFlags();
				 //是否需要切换synword
				RF_Receive();
			}else{
				ERROR_REPORT();
			}
		}
		//关互斥锁
//		xSemaphoreGive(MutexSemaphor);
		taskYIELD();
	}
}
/**
  * @brief  学习任务
  * @param 
  * @retval None
  */
uint32_t studyBit = 0;
void StudyTask(void *pvParameters)
{
 	 
   while(1)
	 {
		if(xTaskNotifyWait(0xffffffffUL,0xffffffffUL,&studyBit,portMAX_DELAY) == pdPASS){// if limits.h is included ULONG_MAX can replace  0xffffffffUL
			//当此任务被解挂且 学习完成
			RF_Init_FSK();
			
			//开互斥锁 不得被其它任务更改 uc_gWifiSendBufferArr 内容
//			xSemaphoreTake(MutexSemaphor,portMAX_DELAY);
			
			switch(studyBit){
				case STUDY_SUCCUESS :
					STUDY_REPORT();
					break;
				case STUDY_FAILED :
					ERROR_REPORT();
					break;
				case STUDY_SEND433:
					break;
				
				default :
					ERROR_REPORT();
					break;
			}
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		}
		//如果没有收到任务通知但解挂了任务那说明在发送学习
//		vTaskSuspend(NULL); //给客户端回馈完后就可以挂起当前任务
		
		//关互斥锁
//		xSemaphoreGive(MutexSemaphor);
		
	  taskYIELD();
	 }
}
/**
  * @brief  定时任务处理任务 基于RTC每5分钟会获取一下网络时间如果任务发生时间小于5分钟就会以任务时间为闹钟
  * @param 
  * @retval None
  */
void TimedTask(void *pvParameters)
{
//	u32 ul_NotifyValue=0;
	u32 ul_diff 	= 0; //difference betwenn timstamp and rtc
	volatile u8 	uc_taskID = 0;
	u16 us_strLen	=	0;
	while(1)
	{
		if(ulTaskNotifyTake(pdTRUE,portMAX_DELAY) == pdPASS){//等待任务通知 进入阻滞态
//有任务需要执行才去判断没有任务执行就不管
			if(RECENT_TASK_UTC != 0){
				if(ul_gUnixTimeStramp > RECENT_TASK_UTC){
					ul_diff = ul_gUnixTimeStramp - RECENT_TASK_UTC;
				}else{
					ul_diff = RECENT_TASK_UTC - ul_gUnixTimeStramp;
				}
				
				if( ul_diff < 20 ){ //如果最近的任务还有20秒就达到了，或者已过了20秒
//查出哪个任务ID
					for(uc_taskID = 0; uc_taskID < TIMINGTASK_LIST_LEN_MAX; uc_taskID++){
//有可能是多个任务都是在这一分钟内执行，所以循环查询哪些需要执行
						if(ul_gTimeTaskUnixTimArr[uc_taskID] == RECENT_TASK_UTC){
//执行当前任务
							ExecuteTask(uc_taskID);
//清空对应任务列表的时间戳
							ul_gTimeTaskUnixTimArr[uc_taskID] = 0;
							
//如果当前任务是非周期执行
							memcpy( &us_strLen, &uc_gTimTaskList2Arr[uc_taskID][2], 2 );
							if((uc_gTimTaskList2Arr[uc_taskID][us_strLen + 4] & 0x80) == 0U ){ // + 4 越过“头部长度”和“str长度” 指向了频次
//删除当前任务列表
								memset(&uc_gTimTaskList2Arr[uc_taskID][0],0,TIMINGTASK_HEAD_LEN_MAX);
								//RECENT_TASK_UTC = 0;
//删除对应的Flash 写0可以不擦除直接写
								ul_diff = 0; //把us_strLen做为Buffer传出Flash写参数里
								STMFLASH_Erase_Word(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * uc_taskID)); //把总长度和头长度清零
//任务计数器减1
								st_gHostInfo.timeTaskCount--;
							}else{//如果当前任务是周期执行
//取出出RTC时时间戳并转换为时间日期等。放在hrtc结构体中
								HAL_RTC_GetTimeUser(&hrtc);
//计算下一次执行时间戳 不计算今日
								CalcSpecificPeriodicTask( uc_taskID, FALSE );
							}
						}
					}
//				CalcMinTimstamp();
				}//else if(ul_diff	<	300){ 
//					if(ul_gUnixTimeStramp < RECENT_TASK_UTC){	//如果本次闹钟响了过后 最近任务小于5分钟就直接定到最近的任务
//						HAL_RTC_SetAlarm_IT(&hrtc , RECENT_TASK_UTC);
//					}
//				}
			}
			CalcMinTimstamp();
		}
		 taskYIELD();
	}
}

//TimerHandle_t xTimer;

//void vTimerCallback( TimerHandle_t pxTimer )
//{
//	ESP_ERS_EN;
//	MCU_LED_DIS;
//}

void KeyTask(void *pvParameters)
{ 
	uint32_t	notifyValue = 0;
	uint8_t i;
	while(1){
		if(xTaskNotifyWait(0,0xffffffffUL,&notifyValue,portMAX_DELAY) == pdPASS){
			if(notifyValue == LONG_PRESS_BIT){
				
				MCU_LED_EN;
				ESP_ERS_DIS;
				EraseDevice();
				//us_vgTim1CountHeart = 15;
				i = 2;
				while(i){
					i--;
					vTaskDelay(6000);
				}
				ESP_ERS_EN;
				MCU_LED_DIS;

			}else if(notifyValue == SHORT_PRESS_BIT){
				ESP_RST_DIS;
				vTaskDelay(100);
				ESP_RST_EN;
			}
		}
		taskYIELD();
	}
}
/**
  * @brief  心跳上报，如果时间到了就上报，但是如果有其它的阻滞，那么等待阻滞再上报
  * @param 
  * @retval None
  */
void HeartBeat(void *pvParameters)
{
	u8  getTimeCount = 14;
	while(1)
	{
//		if(us_vgTim1CountHeart == 0){

				

				if(em_gWifiState == login_ok){ //如果登陆成功了就可以心跳
//心跳发送计数。在收到心跳回复清零。如果超过心跳的次数，重新登陆。
					if(st_gHostInfo.lockInfo == 2U){
						uc_gConStasCount++;
						
						if( uc_gConStasCount == RETRY_MAX ){
							uc_gConStasCount = 0;
							em_gWifiState = dns_founded; //清除登陆OK状态
						}else{
							HEARBEAT();
							getTimeCount++;
							if( getTimeCount == 15){ //5分钟同步一下时间
								getTimeCount = 0;
								vTaskDelay(200);
								Get_Time();
							}
						}
					}
				}else if(em_gWifiState == dns_founded){ //如果没有登陆成功就需要再次登陆
					if(st_gHostInfo.lockInfo == 2U){
						uc_gConStasCount++;
						
						if( uc_gConStasCount == RETRY_MAX ){
							uc_gConStasCount = 0;
							Check_DNS();
							vTaskDelay(50);
						}else{
							LOGIN_NORM();
						}
					}
				}else if(em_gWifiState == connect_router){
					if( uc_gConStasCount == RETRY_MAX ){
						uc_gConStasCount = 0;
						Check_DNS();
					}
				}else if(em_gWifiState == disconnect){
					uc_gConStasCount++;
					
					if( uc_gConStasCount == RETRY_MAX ){
						uc_gConStasCount = 0;
						xTaskNotify(KeyTaskHandle,SHORT_PRESS_BIT,eSetBits); //重启模块
					}else{
						Check_DNS();
					}
					
				}
//				us_vgTim1CountHeart = HEART_TIM;

				

//		}
//		taskYIELD();
		vTaskDelay(20000);
	}
}
/**
  * @brief  空间任务勾子函数用于获取温度和喂狗
  * @param 
  * @retval None
  */
void vApplicationIdleHook( void )
{
//	MCU_LED_EN;
//	delay_xms(500);
//	MCU_LED_DIS;
//	delay_xms(500);
//	HAL_IWDG_Refresh(&hiwdg);
	if(us_vgTim1CountTemp == 0){
		us_vgTim1CountTemp = TEMP_CYCLE;
		portENTER_CRITICAL();
		st_gHostInfo.temp = (Get_Temprate() / 100);
		portEXIT_CRITICAL();
		MCU_LED_DIS;
	}
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
