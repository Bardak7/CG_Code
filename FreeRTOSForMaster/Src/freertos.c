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


vu8  uc_vgTim1CountRetryRF; //�ط�
vu16 us_vgTim1CountHeart;		//����
vu16 us_vgTim1CountTemp;
/* extern variable */
//extern volatile u8	uc_vgTim2CountMode;
extern volatile u8	uc_vgStudyCount;
								u16 us_gAddrToFlashBufArr[3]; //ǰ2��������Ϊ���صĻ�ֻ��ĵ�ַ ����������ڿ��ص�ͨ��bufferд��flash


/**WIFI���Ͷ������**/
#define 	SEND_QUE_LEN	3
QueueHandle_t Queue_Sendout;  

//SemaphoreHandle_t xSemaphore = NULL;//�����ֵ�ź������

						/**���½ṹ������WIFI��ʽ����ȡ������ const ��ָ��������޶����ɸ�����ָ�������**/
const ST_WifiDataPackHead		 			* const	stp_gWifiDataPackHead=(ST_WifiDataPackHead *)USART_RX_BUF;  //st��ʾ����һ���ṹ�������p��ʾ����һ��ָ������_g��ʾ����һ��globalȫ�ֱ���
const ST_Long   					*stp_gLongID;
const ST_IntInt						*stp_gIntInt;
const ST_LongCharChar			*stp_gLongCC;
const ST_LongStirng				*stp_gLongStr;
const ST_LongShortString	*stp_gLongShortStr;
const ST_ShortLong				*stp_gShortLong;


volatile EM_ConState em_gWifiState;

/**����״̬���� �� ���û���������ظ�  �ȵ�**/
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
/**��������ͨ��WIFI*/
void SendWifiTask(void *pvParameters);
#define SEND_WIFI_DEPTH 	256
#define SEND_WIFI_PRIO 		 9
TaskHandle_t  SendWifiTaskHandle;
u8 volatile 	n_gWifiSend;
/*����CMT����*/
void SendCMTTask(void *pvParameters);
#define SEND_CMT_DEPTH			128
#define SEND_CMT_PRIO				6
TaskHandle_t	SendCMTTaskHandle;
/*��������CMT2300������*/
void ReceiveCMTTask(void *pvParameters);
#define	RECEIVE_CMT_DEPTH		128
#define RECEIVE_CMT_PRIO		12
TaskHandle_t	ReceiveCMTTaskHandle;
/*����ѧϰ����*/
void StudyTask(void *pvParameters);
#define STUDY_DEPTH					128
#define STUDY_PRIO					8
TaskHandle_t			StudyTaskHandler;
/**����ʱ�������**/
void TimedTask(void *pvParameters);
#define TIME_TASK_DEPTH			90
#define TIME_TASK_PRIO			7
TaskHandle_t	TimeTaskHandle;

/**�����������**/
void KeyTask(void *pvParameters);
#define KEY_TASK_DEPTH			50
#define KEY_TASK_PRIO				5
TaskHandle_t	KeyTaskHandle;

/**����������أ���**/
void HeartBeat(void *pvParameters);
#define HEART_TASK_DEPTH			50
#define HEART_TASK_PRIO				5

//ST_433Packet static volatile const * stp_g433Pack; //ֻ�ܱ����ļ�ʹ��
/*�����ź������*/
//QueueHandle_t cmtSendQueueHanlde;
/**�����ź������ڻ�����������**/
//QueueHandle_t  MutexSemaphor;

/*��Ϣ���о������RF������Ϣ���ر����ڶ�ʱ��������Ҫ���Ͷ����*/
QueueHandle_t RFSendQueueHandle;  //����Ϣ���д���USART���ݵ�����

/******************************************************************************
 * FunctionName : Wifi_QueueIn
 * Description  : ��wifi���Ͷ�����ӣ���SendWifiTask���Ӳ�ͨ�����ڷ���ESP �ж��в��ɵ��ô˺���
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
	/*����һ����Ϣ��������RF�ķ���*/
	RFSendQueueHandle=xQueueCreate(TIMINGTASK_CMD_MAX,RF433BUFF_MAX);		 //�����3����Ŀ�Ŀ���������һ��ֻ�ܷ���64���ֽڵ�����		
	/*����һ����Ϣ��������RF�ķ���*/
	Queue_Sendout = xQueueCreate(SEND_QUE_LEN,sizeof(ST_Report));
		
	Protocol_Init();
								
	uc_vgTim1CountRetryRF = 50;						
	
  								
	/*����һ���������*/
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
/**����ͷ��ȷ���**/
			if(stp_gWifiDataPackHead->head == PKT_HDR){
				
				//�������� ���ñ������������ uc_gWifiSendBufferArr ����
//			  xSemaphoreTake(MutexSemaphor,10);stp_gWifiDataHeadToSend,
				InitWifiSendHeader(stp_gWifiDataPackHead); //�ѽ��յ��İ�ͷ���Ƶ����͵Ļ���İ�ͷ��ȥ
				if(stp_gWifiDataPackHead->cmd == LOGIN_OK){
					MCU_LED_DIS;
					em_gWifiState = login_ok;
//					us_vgTim1CountHeart = 5; //5*500MS��������
					if(st_gHostInfo.lockInfo == 2U){
						HEARBEAT();
					}
					uc_gConStasCount = 0;
					n64_lAddrBuffer = 0U;
				}else if(stp_gWifiDataPackHead->cmd == HEARTBEAT){
					MCU_LED_DIS;
					n64_lAddrBuffer = 0U;
					uc_gConStasCount = 0;
//					us_vgTim1CountHeart = 46; //46*500MS����û���յ��ͱ��
				}else{
	//���豸ID��ʽ������
					stp_gLongID=(ST_Long*)(&stp_gWifiDataPackHead->next);
					/**�������Ӧ���豸ID**/
					n64_lAddrBuffer=stp_gLongID->hostID1;
					n64_lAddrBuffer<<=32;
					n64_lAddrBuffer|=stp_gLongID->hostID2;  //�������Ӧ���豸ID
				}
				/*******************/
				if(st_gHostInfo.deviceID==n64_lAddrBuffer){ //������͹�����ID�����ǵ�ID����
					
					n64_lAddrBuffer = 0;
					
					switch(stp_gWifiDataPackHead->cmd){
	/**�㲥����************/
						case BROADCAST :
							//InitWifiSendHeader(stp_gWifiDataHeadToSend,stp_gWifiDataPackHead);
							BROADCAST_FUN((u32)(st_gHostInfo.deviceID>>32),(u32)st_gHostInfo.deviceID);//0x50578248,0x066BFF54);//
	//						WifiIntLongCCReport(&stp_gWifiDataHeadToSend->next,0x50578248,0x03,0x07);
						break;
	/**��ӷֻ�����************/					
						case ADD_SLAVE :
							/*Add slave command only has one address,no need to format others data*/
							ADD_SLAVE_FUN();
							
							break;
	/**ѧϰ����************/					
						case STUDY :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							
							STUDY_FUN(stp_gLongCC->studyModeOrChannel);
							break;
	/**ֹͣѧϰ����************/	
						case STOP_STUDY :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							STOP_FUN(RESULT_TRUE);
							break;
	/**��������************/	
						case CONTROL :
							stp_gLongStr=(ST_LongStirng*)(&stp_gLongID->next);
	//						stp_gLongStr->slaveIDL
							n64_lAddrBuffer=stp_gLongStr->slaveIDH;
							n64_lAddrBuffer<<=32;
							n64_lAddrBuffer|=stp_gLongStr->slaveIDL;  //�������Ӧ���豸ID
							if(n64_lAddrBuffer==st_gHostInfo.deviceID){ //����Ƿ�����ǰ�豸�ģ�ֱ�ӽ��봦��
								
								CONTROL_FUN(&stp_gLongStr->content,stp_gLongStr->stringLen);
							}else{
								CONTROL_SLAVE_FUN(stp_gLongStr->slaveIDL,stp_gLongStr->stringLen,&stp_gLongStr->content);
							}
							break;
	/**������������************/	
						case LOCK :
							LOCK_FUN();
							break;
	/**ͨ���ͻ�����������************/	
						case UPDATE_BY_CLIENT :
							stp_gLongShortStr=(ST_LongShortString*)(&stp_gLongID->next);
							UPDATA_FUN();
							break;
	/**��ѯ�豸״̬����************/	
						case QUERY_DEVICE_STATE :
							QUERY_FUN();
							break;
	/**������ѯ�豸״̬����************/	
						case BATCH_QUERY :
							//������ѯ Ӧ���·� ���鳤�� �� �豸ID���� �������а���64λ�豸��ַ�� 
//							stp_gShortLong=(ST_ShortLong*)(&stp_gLongID->next);
							BATCH_QUERY_FUN();
							break;
	/**��Ӷ�ʱ��������************/	
						case ADD_TIMED_TASK:
							
							timeID=ADD_TIME_LIST_FUN(&stp_gLongID->next);
							if(timeID==0){		//ʧ�ܻ᷵��0
								ERROR_REPORT();
							}else{
								ADD_TIME_LIST_REPORT(timeID&0x7F);//���صĶ�ʱ���������ID

							}
							break;
	/**ɾ����ʱ��������************/	
						case DELETE_TIMED_TASK:
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
						
							if(RESULT_FALSE!=DELETE_TIMED_TASK_FUN(&stp_gIntInt->intData2.state,(u8)stp_gIntInt->intData1.taskID)){ //�������ʧ�ܾ�ֱ�ӷ�������ID�ͷ���ֵ
								SUCCESS_REPORT();
							}else{
								ERROR_REPORT();
							}
							break;
	/**���Ķ�ʱ��������************/	
						case MODIFY_TIMED_TASK:
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							if(RESULT_FALSE!=MODIFY_TIMED_TASK_FUN(&stp_gIntInt->intData2.state,stp_gIntInt->intData1.taskID)){
								SUCCESS_REPORT();
							}else{
								ERROR_REPORT();
							}
							break;
	/**��ȡ��ʱ�����б�************/	
						case GET_TIMED_TASK_LIST :
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							GET_TIMED_TASK_LIST_FUN(stp_gIntInt->intData1.startPos,stp_gIntInt->intData2.amount);
							break;
	/**��ȡ������ʱ����************/	
						case GET_ONE_TIMED_TASK :
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							GET_ONE_TIMED_TASK_FUN(stp_gIntInt->intData1.taskID);
							break;
	/**���Ķ�ʱ����״̬����************/	
						case MODIFY_TIMED_TASK_STATE :
							stp_gIntInt=(ST_IntInt*)(&stp_gLongID->next);
							MODIFY_TIMED_TASK_STATE_FUN(stp_gIntInt->intData1.taskID,stp_gIntInt->intData2.state);
						
							break;
	/**��ӿ���************/	
						case ADD_SWITCH :
							ADD_SWITCH_FUN();
						
							break;
	/**��ѯ��������************/	
						case QUERY_SWTICH_STATE :
							//����ÿ��ֻ��ѯһ���������Բ���  ���鳤��  Ӧ�ó�Ϊ1
							stp_gShortLong = (ST_ShortLong *)(&stp_gLongID->next);
							ul_addr = 	stp_gShortLong->usIDLH;
							ul_addr	<<= 16;
 							ul_addr	|=	stp_gShortLong->usIDLL;
							QUERY_SWTICH_STATE_FUN(ul_addr);
							break;
	/**���ƿ�������************/	
						case CONTROL_SWITCH :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							CONTROL_SWITCH_FUN(stp_gLongCC->slaveIDL,&stp_gLongCC->studyModeOrChannel);
							break;
	/**��ȡ������������************/	
						case GET_UPDATE_PROGRESS :
							GET_UPDATE_PROGRESS_FUN();
							break;
	/**ɾ���ֻ�����************/	
						case DELETE_SLAVE :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							DELETE_SLAVE_FUN(stp_gLongCC->slaveIDL);
							break;
	/**ɾ����������************/	
						case DELETE_SWITCH :
							stp_gLongCC=(ST_LongCharChar *)(&stp_gLongID->next);
							DELETE_SWITCH_FUN(stp_gLongCC->slaveIDL);
							break;
					}
				}else if(n64_lAddrBuffer == 0U){//(n64_lAddrBuffer == 0U){  //������͹�����ID��0�Ļ��п����ǵ�һ��������������ʱ������������豸��Ӧ���ظ�
					if(stp_gWifiDataPackHead->cmd == BROADCAST){
						if(st_gHostInfo.lockInfo != 2U){ //���δ��ӱ������� �����ع㲥
							st_gHostInfo.lockInfo = 2U;
							STMFLASH_Write(LOCK_FLAG_FLASH_ADDR,&st_gHostInfo.lockInfo,1);
							InitWifiSendHeader(stp_gWifiDataPackHead);
							BROADCAST_FUN((u32)(st_gHostInfo.deviceID>>32),(u32)st_gHostInfo.deviceID);
						}else{  //��������ʲô������
							;
						}
					}
//					else if(stp_gWifiDataPackHead->cmd == HEARTBEAT){ //�����ظ�
//						;
//					}else if(stp_gWifiDataPackHead->cmd == LOGIN_OK){ //��½�ظ�
//						;
//					}
				}else{  //������Ǳ�����ַ�Ҳ��� Ϊ�յĵ�ַ �Ͳ��Ǳ���ַ��Ӧ������
					MCU_LED_DIS;
					n64_lAddrBuffer = 0;
//					ERROR_REPORT(&stp_gWifiDataHeadToSend->next);					
				}
				//������� ֻҪ��ͷ�Ծ��п�����Ҫ����wifi���ݾͿ������� Ȼ���ٵ�����ػ�����
//				xSemaphoreGive(MutexSemaphor);
			}else{
/**�����ͷ�������ݰ����ܾ���һЩ����״̬��**/
				if(strstr((const char *)USART_RX_BUF,"DNS has been founded") != NULL){
					em_gWifiState=dns_founded;  //����ҵ���DNS��ôһ�����Ѿ�������Ҳ��ȡ��������ʱ���� 
//					ul_gUnixTimeStramp=1540630546;
//					MX_RTC_Init(ul_gUnixTimeStramp);
//					DeviceLogin(1,NULL); //������ҵ���Ӧ��DNS�Ϳ��Ե�½��
//					HAL_RTC_GetTimeUser(&hrtc);
//					us_vgTim1CountHeart=20; //20*500MS������ �������µ�½
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
//					 us_vgTim1CountHeart = HEART_TIM; //���1���Ӻ���û������������һ��WIFI
					 }else if(strstr((const char *)USART_RX_BUF,"time:") != NULL){
		//				 UnixTime=(u32)()
						 //ul_gUnixTimeStramp = 555555;
//					 memcpy(&ul_gUnixTimeStramp,USART_RX_BUF+strlen("time:"),4);//sizeof("time:")-1 ��Ϊ�˼�ȥһ��������
					 ul_gUnixTimeStramp = atoi( (const char*)(USART_RX_BUF + strlen("time:")));
					 // ����ʱ���
					 if(ul_gUnixTimeStramp > COMPILE_TIME){ //ʱ��϶��ǻ���ڱ���ʱ��� ʱ���ֻ���ǲ��½�
						 HAL_RTC_SetTimeUser(&hrtc,ul_gUnixTimeStramp); //У������ʱ��
						 if(uc_fristGetTime == 0){
								uc_fristGetTime = 1;
								CalcAllTask();   // ֻ�ڵ�һ�λ�ȡ�����ʱ��Ű����е������ʱ�����һ��
						 }
						 //ul_gUnixTimeStramp = RTC_ReadTimeCounter(&hrtc);
						 xTaskNotifyGive(TimeTaskHandle);
					 }
					 HAL_RTC_SetAlarm_IT(&hrtc , ul_gUnixTimeStramp + ALARM_CYCLE);
				 }else{//�����ͷ����ȷ���ҷ����ӻ���ʱ��ֱ�ӷ��ش���
//						ERROR_REPORT(&stp_gWifiDataHeadToSend->next);
//					 memset(USART_RX_BUF,0,100);
					 MCU_LED_DIS;
				 }
				MCU_LED_DIS;				 
			}
//			portENTER_CRITICAL();
//			memset(USART_RX_BUF,0,USART_REC_LEN); //��������usart���ջ���
//			portEXIT_CRITICAL();
		}
		taskYIELD();
	}

}
/**
  *SendWifiTask
	*��������֪ͨ��ȡ����Ҫ���͵�����ʱͨ��DMA���͵�USART1 û�л�ȡ������֪ͨ��ʱ���������̬
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
					/**���������ݰ�ʱ�����������**/
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
//			while(__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_TC) != SET){		//�ȴ����ͽ���
//				vTaskDelay(20);
//			}
////			portENTER_CRITICAL();
//			HAL_UART_Transmit_DMA(&UART1_Handler, (uint8_t *)stp_Report->reportBuf, stp_Report->reportLen);//��������
//////			portEXIT_CRITICAL();
////			//ʹ�ܴ���1��DMA���� //�ȴ�DMA������ɣ���ʱ������������һЩ�£����
////		    //ʵ��Ӧ���У����������ڼ䣬����ִ�����������
//		    while(1)
//		    {
//					if(__HAL_DMA_GET_FLAG( &UART1TxDMA_Handler, DMA_FLAG_TC4 ) )//�ȴ�DMA1ͨ��4�������
//					{
//							__HAL_DMA_CLEAR_FLAG( &UART1TxDMA_Handler, DMA_FLAG_TC4) ;//���DMA1ͨ��4������ɱ�־
//							HAL_UART_DMAStop(&UART1_Handler);      //��������Ժ�رմ���DMA
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
////  printf("Period Timer is runing!\r\n"); //������ص������в������������API
//	
//	if(uc_vgTim2CountMode == 1){ //�������ѧϰ״̬
//		uc_vgStudyCount++;
//		if(uc_vgStudyCount>STUDY_TIME_MAX){ //�������ѧϰ�����ʱ����ôѧϰʧ��
//			//�ر�ѧϰ�õĶ�ʱ��2 50us����ѧϰ����
//			__HAL_TIM_DISABLE(&htim2);
//			//���ѧϰ���񣬷���ʧ��״̬λ 
//		}
//	}
//}*/
/**
  * @brief  SendCMTTaskTask ��ȡ��Ϣ�������� ��static��������Ҫ���͵�ID���ID����һ����ͬ ���ͼ�����һ
  * @param  None
  * @retval None
  */
static u8 uc_sgRFSendCount; //���ͼ���
static u8 uc_sgSendID; //��һ�η��͵�ID �ޱ��ļ�����
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
		if(RFSendQueueHandle != NULL){ //���RF���͵���Ϣ���д����ɹ�
			if(xQueuePeek(RFSendQueueHandle,uc_gRXBuffer,portMAX_DELAY)){ //��Ϣ���е���CMT��Ҫ���͵����� ��������
				stp_lSend433Pack=(ST_433Packet *)uc_gRXBuffer;    //��ʽ��
				
				if( uc_sgSendID != stp_lSend433Pack->Id ){ //��������ڵ�ǰID�Ļ�˵����һ���ѷ�����ɻ���ʧ���������Ϣ�����е��µ�����
					uc_sgSendID = stp_lSend433Pack->Id;
					if( stp_lSend433Pack->Id == st_gHostInfo.deviceID ){ //��ʱ��������м�����������Ҫ�������м�����Ҫ�ӷֻ�����ȥ
						
					}else{
						uc_sgRFSendCount++;
						/***�˲��ִ������CMT��Ҫ���͵Ĵ�������Ƿֻ��ͷ���40������ǿ��ؾͷ���20��***/
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
						//ֱ�ӷ���
						RF_Send(uc_gRXBuffer,stp_lSend433Pack->Len+12);
						uc_vgTim1CountRetryRF=(rand()%5)+1;  //ȡ1��4�����ֵ (1~4)*120=(120~480)ms;�ĵ÷�ʱ��
					}
				}else{  //����ǵڶ��η���
					if(uc_sgRFSendCount>RF433SEND_MAX){ //�����������ʹ�����
						MCU_LED_DIS;
						SET_DEFAULT_SYNC();
						uc_sgRFSendCount=0; 							//���㷢�ͼ���
						xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //�����Ϣ����
					}else{
						//�ȴ�����֪ͨ������  �����ȡһ��������� ��ʱ��1��֪ͨ
						ul_lNotifyValue=ulTaskNotifyTake(pdTRUE,0);
						if(ul_lNotifyValue==1){
							ul_lNotifyValue = 0;
							RF_Send(uc_gRXBuffer,stp_lSend433Pack->Len+12);
							uc_sgRFSendCount++; //��¼�ѷ���һ�� //unsigned char static global�ı���
							uc_vgTim1CountRetryRF=(rand()%5)+1;  //ȡ1��4�����ֵ (1~4)*120=(120~480)ms;���ط�ʱ��							
						}	
					}
				}
			}
		}

		taskYIELD();
	}

}
/**
  * @brief  ����CMT2300������   ���չ�����Ҫ����   ���������1�����յ������ݡ�2�����յ��˷��ͳɹ�����Ϣ
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
		
		//�������� ���ñ������������ uc_gWifiSendBufferArr ����
//			xSemaphoreTake(MutexSemaphor,portMAX_DELAY);
		
		if(xTaskNotifyWait(0,0xffffffffUL,&notifyValue,portMAX_DELAY) == pdPASS){ //���յ�CMT2300A�����ݻ��ͳɹ�ǰ��������̬
			if(notifyValue==CMT_RECEIVEBIT){
				RF_Read(uc_sgRFReadBuffer,RF433BUFF_MAX);
				//��RF���յ������ݸ�ʽ��
				stp_lRead433Pack=(ST_433Packet *)uc_sgRFReadBuffer;
				
				if(stp_lRead433Pack->TargetADDR==((u32)st_gHostInfo.deviceID)){  //Ŀ���ַ�ǲ��Ǳ�����ַ �ɲ��ж� ��synword�ѹ���
					switch(stp_lRead433Pack->Cmd-0x80)
					{
	/*******�����ɿͻ��˿��ƺ��ϱ�********/
						case RF_SWITCH_CONTROL :
							//ǿ��ת��ȡ�����ص�״̬��·��
							stp_lSwitchInfo=(ST_SwitchState *)(&stp_lRead433Pack->Data); 
						
							for(i=0;i<SWITCH_LIST_MAX;i++){
								if(st_gHostInfo.stSwitch[i].switchID== stp_lRead433Pack->SoucreAddr){ //���Դ��ַ��ĳ�����صĻ�
									st_gHostInfo.stSwitch[i].state	=stp_lSwitchInfo->switchState;
									st_gHostInfo.stSwitch[i].channle=stp_lSwitchInfo->switchChannel;
									break;
								}
							}
							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM_SWITCH); //ȷ�ϵ�ʱ��ֱ��CMT2300A����һ��
							/********���´���ͨ��WIFI���ظ��ͻ���*********/
							uc_sgRFSendCount=0; 							//���㷢�ͼ���
							uc_vgTim1CountRetryRF=0;					//����Ҫ���ط�
							xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //�����Ϣ����
							CONTROL_SWITCH_REPORT(stp_lSwitchInfo->switchState);
							/****************************************/
							break;
	/********���������ϱ�***����Ҫ���ظ��ͻ���******/
						case RF_SWITCH_REPORT :
							stp_lSwitchInfo=(ST_SwitchState *)(&stp_lRead433Pack->Data); 
							 
							for(i=0;i<SWITCH_LIST_MAX;i++){
								if(st_gHostInfo.stSwitch[i].switchID== stp_lRead433Pack->SoucreAddr){ //���Դ��ַ��ĳ�����صĻ�
									st_gHostInfo.stSwitch[i].state	=stp_lSwitchInfo->switchState;      //����״̬
									//st_gHostInfo.stSwitch[i].channle=stp_lSwitchInfo->switchChannel;
									break;
								}
							}
							if(i>=SWITCH_LIST_MAX){
								 //���û��������ؾ͸�һ��ɾ��������ص����� ͨ����������
							}else{
								//����Ϊʲô��TargetADDR ��CMT���͵�ʱ���������Ϊsync word��
								Confirm(stp_lRead433Pack->TargetADDR,RF_CONFIRM); //ȷ�ϵ�ʱ��ֱ��CMT2300A����һ��
							}
							/**********��ͻ��˷��سɹ�����************/
							//���������ϱ�����Ҫ��ͻ��˷���
	//						WifiIntReport(&stp_gWifiDataHeadToSend->next,1); //
	//						len=findLen((u8*)uc_gWifiSendBufferArr);
	//						if(len>10){
	//							xTaskNotify(SendWifiTaskHandle,(u32 )len,eSetValueWithOverwrite); //Wifi����						
	//						}
							/*************************/
							break;
	/********��ӷֻ���Ļ���*********/
						case RF_ADD_SLAVE:
							for(i=0;i<SLAVE_LIST_MAX;i++){
								if(stp_lRead433Pack->SoucreAddr == st_gHostInfo.stSlave[i].slaveID){
									break;
								}
							}if(i>=SLAVE_LIST_MAX){
							  for(i=0;i<SLAVE_LIST_MAX;i++){
									if(st_gHostInfo.stSlave[i].slaveID ==0){
										st_gHostInfo.stSlave[i].slaveID = stp_lRead433Pack->SoucreAddr;
										//��ID�浽FLASH��ȥ
										us_gAddrToFlashBufArr[0]=(u16)st_gHostInfo.stSlave[i].slaveID; //�ʹ�ͣ��ߴ��
										us_gAddrToFlashBufArr[1]=(u16)(st_gHostInfo.stSlave[i].slaveID>>16);
										STMFLASH_Write(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),us_gAddrToFlashBufArr,2);
//										STMFLASH_Write(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),(u16 *) &st_gHostInfo.stSlave[i].slaveID,2);
										st_gHostInfo.slaveCount++;
										break;
									}
								}
								if(i>=SLAVE_LIST_MAX){ //������������еķֻ�ID���еĻ���ô�Ѵﵽ���ֻ������  ���Ը��ǻ��߱���
									//����ѡ�񲻱���
									st_gHostInfo.stSlave[SLAVE_LIST_MAX-1].slaveID = stp_lRead433Pack->SoucreAddr; //�������һ��
									STMFLASH_Write(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),
																(u16 *)&st_gHostInfo.stSlave[SLAVE_LIST_MAX-1].slaveID,
																2);
									st_gHostInfo.slaveCount++;
								}
							}
							
							/******�ѽ��յ����������һ�εĶ��кͷ��ʹ�������************/
							//Cmt2300_SetSynWord((u32)(st_gHostInfo.deviceID)); //��synWord��Ϊ������ַ
//							uc_sgRFSendCount=0; 							//���㷢�ͼ���
//							uc_vgTim1CountRetryRF=0;					//����Ҫ���ط�
//							xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //�����Ϣ����
							delay_ms(100);
							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM); //ȷ�ϵ�ʱ��ֱ��CMT2300A����һ��
							delay_ms(100);
							SET_DEFAULT_SYNC();
							/**���ظ��ͻ���*/
							ADD_SLAVE_REPORT(stp_lRead433Pack->SoucreAddr);
							break;
	/********��ӿ��غ�Ļ���*********/						
						case RF_ADD_SWITCH :
							stp_lSwitchInfo=(ST_SwitchState *)(&stp_lRead433Pack->Data); 
							for(i=0;i<SWITCH_LIST_MAX;i++){
								if(stp_lRead433Pack->SoucreAddr == st_gHostInfo.stSwitch[i].switchID){ //����ӹ��Ͳ��ù���
									break;
								}
							}
							if(i>=SWITCH_LIST_MAX){ //���û����ӹ�����ӵ��б��flash��ȥ
								for(i=0;i<SWITCH_LIST_MAX;i++){
									if(st_gHostInfo.stSwitch[i].switchID ==0){
										st_gHostInfo.stSwitch[i].switchID = stp_lRead433Pack->SoucreAddr;
										st_gHostInfo.stSwitch[i].channle	= stp_lSwitchInfo->switchChannel;
										st_gHostInfo.stSwitch[i].state		= stp_lSwitchInfo->switchState;	
										//��ID�浽FLASH��ȥ
										us_gAddrToFlashBufArr[0] = (u16)st_gHostInfo.stSwitch[i].switchID; //�ʹ�ͣ��ߴ��
										us_gAddrToFlashBufArr[1] = (u16)(st_gHostInfo.stSwitch[i].switchID>>16);
										us_gAddrToFlashBufArr[2] = (u16)st_gHostInfo.stSwitch[i].channle;
										STMFLASH_Write(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i),us_gAddrToFlashBufArr,3); //д��flash
										st_gHostInfo.switchCount++;
										break;
									}
								}
								if(i>=SWITCH_LIST_MAX){ //����б�����ֱ�Ӹ������һ����
									st_gHostInfo.stSwitch[SLAVE_LIST_MAX-1].switchID	= stp_lRead433Pack->SoucreAddr; //�������һ��
									st_gHostInfo.stSwitch[SLAVE_LIST_MAX-1].channle		= stp_lSwitchInfo->switchChannel;
									st_gHostInfo.stSwitch[SLAVE_LIST_MAX-1].state			= stp_lSwitchInfo->switchState;	
									//��ID�浽FLASH��ȥ
									us_gAddrToFlashBufArr[0] = (u16)st_gHostInfo.stSwitch[SLAVE_LIST_MAX - 1].switchID; //�ʹ�ͣ��ߴ��
									us_gAddrToFlashBufArr[1] = (u16)(st_gHostInfo.stSwitch[SLAVE_LIST_MAX - 1].switchID>>16);
									us_gAddrToFlashBufArr[2] = (u16)st_gHostInfo.stSwitch[SLAVE_LIST_MAX - 1].channle;
									STMFLASH_Write(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * (SLAVE_LIST_MAX - 1)),us_gAddrToFlashBufArr,3); //д��flash
//									st_gHostInfo.switchCount++;
								}
							}
							/**���ظ�����*/
//							uc_sgRFSendCount=0; 							//���㷢�ͼ���
//							uc_vgTim1CountRetryRF=0;					//����Ҫ���ط�
//							xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //�����Ϣ����
							delay_ms(100);
							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM); //ȷ�ϵ�ʱ��ֱ��CMT2300A����һ��
							delay_ms(100);
//							Confirm(stp_lRead433Pack->SoucreAddr,RF_CONFIRM); //ȷ�ϵ�ʱ��ֱ��CMT2300A����һ��
							/**���ظ��ͻ���*/
							ADD_SWITCH_REPORT(stp_lRead433Pack->SoucreAddr,stp_lSwitchInfo->switchChannel,stp_lSwitchInfo->switchState);
							
							break;
	/********���Ʒֻ���Ļ���*********/					
						case RF_CONTROL_DEVICE :
							
							CONTROL_REPORT(stp_lRead433Pack->Data); //���ظ��ͻ��˳ɹ��Ϳ��ƺ��״̬
							break;
	/********ɾ���ֻ���Ļ���*********/					
						case RF_DEL_SLAVE :
							//�ڷ���RF֮ǰ��Ҫ��鱾���Ƿ�������ֻ������û�о�ֱ�ӻظ��ɹ�
							for(i=0;i<10;i++){
								if(st_gHostInfo.stSlave[i].slaveID == stp_lRead433Pack->SoucreAddr){
									st_gHostInfo.stSlave[i].slaveID=0U; //ɾ���ֻ�����
									
									//��Flash�еķֻ��洢ɾ��
									/**
									*/
									us_gAddrToFlashBufArr[0] = 0; //ֱ��д0�ɵ���STMFLASH_Write_NoCheck ����CPUռ��
									us_gAddrToFlashBufArr[1] = 0;
									STMFLASH_Erase_Word(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i));
									if(st_gHostInfo.slaveCount > 0)
											st_gHostInfo.slaveCount--;
									DELETE_SLAVE_REPORT(RESULT_TRUE);
									break; //�˳�for
								}
							}
							DELETE_SLAVE_REPORT(RESULT_TRUE); //���ظ��ͻ��˳ɹ�
							break;//�˳�case
	/********ɾ�����غ�Ļ���*********/					
						case RF_DEL_SWITCH :
							for(i=0;i<10;i++){
								if(st_gHostInfo.stSwitch[i].switchID == stp_lRead433Pack->SoucreAddr){
									st_gHostInfo.stSwitch[i].switchID	=0;
									st_gHostInfo.stSwitch[i].channle	=0;
									st_gHostInfo.stSwitch[i].state		=0;  //���㿪�ص�����
									
									//��FLASH�еĿ��ش洢ɾ��
									/**
									*/
									us_gAddrToFlashBufArr[0] = 0; //ֱ��д0�ɵ���STMFLASH_Write_NoCheck ����CPUռ��
									us_gAddrToFlashBufArr[1] = 0;
									STMFLASH_Erase_Word(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i)); //д��flash
									st_gHostInfo.switchCount--;
									DELETE_SWITCH_REPORT(RESULT_TRUE);//���ظ��ͻ��˳ɹ�
									break;//�˳�for
								}
							}
							break; //�˳�case
					}
					MCU_LED_DIS;
					uc_sgRFSendCount=0; 							//���㷢�ͼ���
					uc_vgTim1CountRetryRF=0;					//����Ҫ���ط�
					xQueueReceive(RFSendQueueHandle,uc_gRXBuffer,0); //�����Ϣ����
					SET_DEFAULT_SYNC();
				}
			}else if(notifyValue==CMT_SENDBIT){ //���ͳɹ�
				Cmt2300_ClearInterruptFlags();
				 //�Ƿ���Ҫ�л�synword
				RF_Receive();
			}else{
				ERROR_REPORT();
			}
		}
		//�ػ�����
//		xSemaphoreGive(MutexSemaphor);
		taskYIELD();
	}
}
/**
  * @brief  ѧϰ����
  * @param 
  * @retval None
  */
uint32_t studyBit = 0;
void StudyTask(void *pvParameters)
{
 	 
   while(1)
	 {
		if(xTaskNotifyWait(0xffffffffUL,0xffffffffUL,&studyBit,portMAX_DELAY) == pdPASS){// if limits.h is included ULONG_MAX can replace  0xffffffffUL
			//�������񱻽���� ѧϰ���
			RF_Init_FSK();
			
			//�������� ���ñ������������ uc_gWifiSendBufferArr ����
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
		//���û���յ�����֪ͨ�������������˵���ڷ���ѧϰ
//		vTaskSuspend(NULL); //���ͻ��˻������Ϳ��Թ���ǰ����
		
		//�ػ�����
//		xSemaphoreGive(MutexSemaphor);
		
	  taskYIELD();
	 }
}
/**
  * @brief  ��ʱ���������� ����RTCÿ5���ӻ��ȡһ������ʱ�����������ʱ��С��5���Ӿͻ�������ʱ��Ϊ����
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
		if(ulTaskNotifyTake(pdTRUE,portMAX_DELAY) == pdPASS){//�ȴ�����֪ͨ ��������̬
//��������Ҫִ�в�ȥ�ж�û������ִ�оͲ���
			if(RECENT_TASK_UTC != 0){
				if(ul_gUnixTimeStramp > RECENT_TASK_UTC){
					ul_diff = ul_gUnixTimeStramp - RECENT_TASK_UTC;
				}else{
					ul_diff = RECENT_TASK_UTC - ul_gUnixTimeStramp;
				}
				
				if( ul_diff < 20 ){ //��������������20��ʹﵽ�ˣ������ѹ���20��
//����ĸ�����ID
					for(uc_taskID = 0; uc_taskID < TIMINGTASK_LIST_LEN_MAX; uc_taskID++){
//�п����Ƕ������������һ������ִ�У�����ѭ����ѯ��Щ��Ҫִ��
						if(ul_gTimeTaskUnixTimArr[uc_taskID] == RECENT_TASK_UTC){
//ִ�е�ǰ����
							ExecuteTask(uc_taskID);
//��ն�Ӧ�����б��ʱ���
							ul_gTimeTaskUnixTimArr[uc_taskID] = 0;
							
//�����ǰ�����Ƿ�����ִ��
							memcpy( &us_strLen, &uc_gTimTaskList2Arr[uc_taskID][2], 2 );
							if((uc_gTimTaskList2Arr[uc_taskID][us_strLen + 4] & 0x80) == 0U ){ // + 4 Խ����ͷ�����ȡ��͡�str���ȡ� ָ����Ƶ��
//ɾ����ǰ�����б�
								memset(&uc_gTimTaskList2Arr[uc_taskID][0],0,TIMINGTASK_HEAD_LEN_MAX);
								//RECENT_TASK_UTC = 0;
//ɾ����Ӧ��Flash д0���Բ�����ֱ��д
								ul_diff = 0; //��us_strLen��ΪBuffer����Flashд������
								STMFLASH_Erase_Word(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * uc_taskID)); //���ܳ��Ⱥ�ͷ��������
//�����������1
								st_gHostInfo.timeTaskCount--;
							}else{//�����ǰ����������ִ��
//ȡ����RTCʱʱ�����ת��Ϊʱ�����ڵȡ�����hrtc�ṹ����
								HAL_RTC_GetTimeUser(&hrtc);
//������һ��ִ��ʱ��� ���������
								CalcSpecificPeriodicTask( uc_taskID, FALSE );
							}
						}
					}
//				CalcMinTimstamp();
				}//else if(ul_diff	<	300){ 
//					if(ul_gUnixTimeStramp < RECENT_TASK_UTC){	//��������������˹��� �������С��5���Ӿ�ֱ�Ӷ������������
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
  * @brief  �����ϱ������ʱ�䵽�˾��ϱ���������������������ͣ���ô�ȴ��������ϱ�
  * @param 
  * @retval None
  */
void HeartBeat(void *pvParameters)
{
	u8  getTimeCount = 14;
	while(1)
	{
//		if(us_vgTim1CountHeart == 0){

				

				if(em_gWifiState == login_ok){ //�����½�ɹ��˾Ϳ�������
//�������ͼ��������յ������ظ����㡣������������Ĵ��������µ�½��
					if(st_gHostInfo.lockInfo == 2U){
						uc_gConStasCount++;
						
						if( uc_gConStasCount == RETRY_MAX ){
							uc_gConStasCount = 0;
							em_gWifiState = dns_founded; //�����½OK״̬
						}else{
							HEARBEAT();
							getTimeCount++;
							if( getTimeCount == 15){ //5����ͬ��һ��ʱ��
								getTimeCount = 0;
								vTaskDelay(200);
								Get_Time();
							}
						}
					}
				}else if(em_gWifiState == dns_founded){ //���û�е�½�ɹ�����Ҫ�ٴε�½
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
						xTaskNotify(KeyTaskHandle,SHORT_PRESS_BIT,eSetBits); //����ģ��
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
  * @brief  �ռ������Ӻ������ڻ�ȡ�¶Ⱥ�ι��
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
