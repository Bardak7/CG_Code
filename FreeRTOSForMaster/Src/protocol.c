#include "protocol.h"
#include "string.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "stmflash.h"
#include <stdlib.h>
#include "rtc.h"

ST_HostInfo	st_gHostInfo;

extern  const volatile ST_RFIRStudy 	 st_gRFStudyData; //����ṹ���ڱ��ļ����ǲ��ɸ��ĵġ�

static ST_Report st_gWifiReport;

void Extract_Time(ST_Time* time,const u8 *timePos);

u16 findLen(const u8 * source );

						/**���½ṹ������WIFI��ʽ�����͵�����**/
ST_WifiDataPackHead		*const	stp_gWifiDataHeadToSend = (ST_WifiDataPackHead 	*)st_gWifiReport.reportBuf; //��ͷ��ָ����Զָֻ��������ĵ�һ��λ�� const��ָ����ָ����ָ���ɱ�
ST_IntIntS			*	volatile			stp_gIntIntSend;//			=	(ST_IntIntS		*)&stp_gWifiDataHeadToSend->next;//�ڶ���ʱǿ��ת��������һ��const����
ST_Int					*	volatile			stp_gIntSend;
ST_IntChar			*	volatile			stp_gIntCharSend;
ST_IntShort			*	volatile			stp_gIntShortSend;
ST_IntLong			*	volatile			stp_gIntLongSend;
ST_IntLongCC		*	volatile			stp_gIntLongCCSend;
ST_IntString		*	volatile			stp_gIntString;
ST_Array_LCSCC	*	volatile			stp_gArr_LCSCC; //

/**
	* ��������ֻ�洢����ͷ��  ��������
	*  			ͷ������(short)  �����ͷ������ֻ����ͷ�����ݡ��ĳ��ȣ�������ͷ�������short������״̬��Byte
	*				ͷ������(���Ͻṹ) ���� Ƶ�� ʱ�� ���� 
	*				����״̬(Byte)
**/
u8 uc_gTimTaskList2Arr[TIMINGTASK_LIST_LEN_MAX][TIMINGTASK_HEAD_LEN_MAX]; //
/*��Ϊ��ʱ�����Flash��ȡ���������*/
u8 uc_gTimCmdArr[TIMINGTASK_DATALEN_MAX];
/**
	* ��������ִ��ʱ���Ӧ��ʱ��������û����������û��ʹ�ܾ�Ϊ0 �±���ID ��Ӧ +1 �����Ǹ���Ϊ�˱������ֵ
**/
u32 ul_gTimeTaskUnixTimArr[TIMINGTASK_LIST_LEN_MAX+1]; //���涨ʱ����ת�����unixʱ��

SemaphoreHandle_t MutexSemaphor;

/******************************************************************************
 * FunctionName : Protocol_Init
 * Description  : 
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void Protocol_Init(void)
{
	MutexSemaphor = xSemaphoreCreateMutex();//xSemaphoreCreateBinary();
}

/**
  * @brief  Extract_Time �ѿͻ��˵��ַ���ʱ��ת������ֵ
  * @param 	time ��ST_Time���͵Ľṹ��ָ�� ����ת�������ֵ����
	*					
	*					timePos��ʱ�俪ʼλ��
  * @retval None
  */
void Extract_Time(ST_Time* time,const u8 *timePos)//u8 * datePos,
{
  u16 us_timeLen;
	us_timeLen = *(u16 *)timePos;
	timePos += 2; //Խ��
	time->Time.Hours			=	(u8)(atoi((const char*)timePos));					//ָ��ָ�����ʱ���Сʱ
	time->Time.Minutes		=	(u8)(atoi((const char*)(timePos+=3)));  	//ָ��ָ�����ʱ��ķ�
	if(us_timeLen == 5){
		time->Time.Seconds	=	0;
	}else{
		time->Time.Seconds	=	(u8)(atoi((const char*)(timePos+=3)));
	}
	timePos+=2;  	//ָ������String(len)
	timePos+=2;		//ָ�����ڵ���
	time->date.Year=atoi((const char*)timePos);
	time->date.Month=(u8)atoi((const char*)(timePos+=5));		//ָ�����ڵ���
	time->date.Date=(u8)atoi((const char*)(timePos+=3));		//ָ�����ڵ���
	
}
/**
  * @brief  findLen �ӽ��յ������� �Ӹ�����λ���ҵ������ 0x21 0x71������
  * @param 	source	��Ҫ���ҵ���ʼ��ַ
  * @retval u16 		����ʼ��ַ�� 0x7121�ĳ��ȣ�����0x7121;
  */
u16 findLen(const u8 * source )
{
  u16 counter=0;
	while(1){
		
		counter++;
//	  if(* source==0x71 ){//&& (*source--)==0x21){ //&&���ϣ��������0x71���ټ����ұ�
//			if(*(source-1) ==0x21){
//		   return counter;
//			}else{
//			 source+=2;
//			}
//		}	
		
		if(* source==0x21){			
			counter|=0x8000;
		}else if(* source==0x71){
			if((counter & 0x8000)!=0){
				return (counter & (~0x8000));
			}
		}
		source++;
		if((counter & 0x7FFF)>WIFI_SEND_BUF_MAX){
		  return 0x0000;
		}
	}
}

/*�����������ʽ���� 64λ������*/
/**
  * @brief  Get long long data put it into addr;
  * @param  start is first pointer of long long data.The normal parameter is the next member of ST_WifiDataPackHead structure
	*					addr to save the long long data ,it's usually be device's address
  * @retval None
  */
void ProcessLong(u8 * start)
{
  const ST_Long * st_lAddr;
	uint64_t addr;
	
	st_lAddr=(ST_Long *)start;
	addr=st_lAddr->hostID2;
	addr<<=32;
	addr|=st_lAddr->hostID1;
}

/**
  * @brief  InitWifiSendHeader copy the header of receive buffer to the send buffer
  * @param 	ST_WifiDataPackHead  * const headerSend,headerSend is Send Buffer
	*					headerReceive is Receive Buffer
  * @retval None
  */
void InitWifiSendHeader(const ST_WifiDataPackHead *const headerReceive)
{
		memcpy(stp_gWifiDataHeadToSend,headerReceive,sizeof(ST_WifiDataPackHead));
		stp_gWifiDataHeadToSend->reserveH = 0;
		stp_gWifiDataHeadToSend->reserveL = 0;
}

/******************************************************************************
 * FunctionName : Get_Time
 * Description  : ��ESP8266��ȡsntpʱ�� �˺����������ж��б�����
 * Parameters   :	none
 * Returns      : ���� RESULT_FALSE���ʧ�ܡ�RESULT_TRUE �ɹ�
*******************************************************************************/
u8 Get_Time(void)
{
	u8 res;
//	while(st_gWifiReport.dataLock){
//		vTaskDelay(20);
//	}
	xSemaphoreTake(MutexSemaphor,10);
	
//	st_gWifiReport.dataLock = 1;
	memset(st_gWifiReport.reportBuf, 0, st_gWifiReport.reportLen);
	sprintf((char *)st_gWifiReport.reportBuf,"Get_time");
	st_gWifiReport.reportLen	=	sizeof("Get_time");
	
	res=Wifi_QueueIn( &st_gWifiReport );
	
//	st_gWifiReport.dataLock = 0;
	xSemaphoreGive(MutexSemaphor);
	return res;
}	
/******************************************************************************
 * FunctionName : Check_DNS
 * Description  : ��ESP8266��ȡ����״̬ �˺����������ж��б�����
 * Parameters   :	none
 * Returns      : ���� RESULT_FALSE���ʧ�ܡ�RESULT_TRUE �ɹ�
*******************************************************************************/
u8 Check_DNS(void)
{
	u8 res;
//	while(st_gWifiReport.dataLock){
//		vTaskDelay(20);
//	}
	xSemaphoreTake(MutexSemaphor,10);
	
//	st_gWifiReport.dataLock = 1;
	memset(st_gWifiReport.reportBuf, 0, st_gWifiReport.reportLen);
	sprintf((char *)st_gWifiReport.reportBuf,"Check_DNS");
	st_gWifiReport.reportLen	=	sizeof("Check_DNS");
	
	res=Wifi_QueueIn( &st_gWifiReport );
	
//	st_gWifiReport.dataLock = 0;
	xSemaphoreGive(MutexSemaphor);
	return res;
}
/**
  * @brief 	WifiIntLongReport ���ظ��ͻ��� INT +LONG
  * @param 	next 	ָ��ָ���ͷ���������	
	*					idH ��4λ  idL ��4λ
  * @retval None
  */
void WifiIntLongReport(u32 idH,u32 idL)
{
  xSemaphoreTake(MutexSemaphor,10);
	stp_gIntLongSend 					=	(	ST_IntLong *)( &stp_gWifiDataHeadToSend->next );
	stp_gIntLongSend->result	=	RESULT_TRUE;
	stp_gIntLongSend->idH			=	idH;				//(u32)(id>>32);	//(st_gHostInfo.deviceID);
	stp_gIntLongSend->idL			=	idL;				//(u32)id;				//st_gHostInfo.deviceID;
	stp_gIntLongSend->epilog	=	EPILOG;
	st_gWifiReport.reportLen	=	findLen( (u8*)st_gWifiReport.reportBuf );
	
	if( st_gWifiReport.reportLen	>	10 ){
			Wifi_QueueIn( &st_gWifiReport );
		}
	xSemaphoreGive(MutexSemaphor);
}

/**
  * @brief  ���سɹ���ʧ�ܵĸ��ͻ��ˣ���������״̬�ȡ�
  * @param next 	ָ��ָ���ͷ���������	
	*				 result	��Ҫ���ظ��ͻ��˵Ľ��	RESULT_TRUE���ɹ� RESULT_FALSE�� ʧ�� �������ز��ô�����ظ�
  * @retval None
  */
void WifiIntReport(u32 result)
{
	xSemaphoreTake(MutexSemaphor,10);
	stp_gIntSend							=	(ST_Int*)( &stp_gWifiDataHeadToSend->next );
	stp_gIntSend->result			=	result;
	stp_gIntSend->epilog			=	EPILOG;
	st_gWifiReport.reportLen	=	findLen( (u8*)st_gWifiReport.reportBuf );
	
	if( st_gWifiReport.reportLen > 10 ){
		Wifi_QueueIn( &st_gWifiReport );//Wifi����						
	}
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  WifiIntIntReport
  * @param 	
	*					id 		����ID
  * @retval None
  */
void WifiIntIntReport(u32 id)
{
	xSemaphoreTake(MutexSemaphor,10);
	stp_gIntIntSend						=	(ST_IntIntS *)( &stp_gWifiDataHeadToSend->next );
	stp_gIntIntSend->result		=	RESULT_TRUE; //��������������һ�����ѳɹ�ִ����
	stp_gIntIntSend->taskID		=	id;
	stp_gIntIntSend->epilog		=	EPILOG; 		
	st_gWifiReport.reportLen	=	findLen( (u8*)st_gWifiReport.reportBuf );
	
	if( st_gWifiReport.reportLen > 10 ){
		Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  ���ظ��ͻ���һ��4���ֽڵ�result��1���ֽڵ�״̬   ���ƿ���	�����豸	�豸����	
  * @param 	next 	ָ��ָ���ͷ���������	
	*					state ��Ҫ���ص�״̬
  * @retval None
  */
void WifiIntCharReport(u8 state)
{
	xSemaphoreTake(MutexSemaphor,10);
	stp_gIntCharSend					=	(ST_IntChar*)( &stp_gWifiDataHeadToSend->next );
	stp_gIntCharSend->result	=	RESULT_TRUE;
	stp_gIntCharSend->state		=	state;
	stp_gIntCharSend->epilogl	=	0x21;
	stp_gIntCharSend->epilogh	= 0x71;
	st_gWifiReport.reportLen	=	findLen( (u8*)st_gWifiReport.reportBuf );
	
	if(st_gWifiReport.reportLen>10){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	xSemaphoreGive(MutexSemaphor);
}

/**
  * @brief  ������ӿ�������  ������ӿ��غͷֻ�
  * @param 	next ��������Ԫ��
	*					addr ��ӿ��غͷֻ�ֻ��Ҫһ��32λ�ĵ�ַ������Ҫ64λ��ַ����λ
	*					channel ͨ����
	*					state	״̬
  * @retval None
  */
void WifiIntLongCCReport(u32 addr,u8 channel,u8 state)
{
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntLongCCSend					=	(ST_IntLongCC *)(	&stp_gWifiDataHeadToSend->next	);
	stp_gIntLongCCSend->result	=	RESULT_TRUE;
	stp_gIntLongCCSend->idH			=	0;
	stp_gIntLongCCSend->idL			=	addr; //��������һ��64λ��С��ת�� idH �� idL�Ի���һ�¡�
	stp_gIntLongCCSend->channle	=	channel;
	stp_gIntLongCCSend->state		=	state;
	stp_gIntLongCCSend->epilog	=	EPILOG;
	st_gWifiReport.reportLen		=	findLen((u8*)st_gWifiReport.reportBuf);
	
	if( st_gWifiReport.reportLen > 10 ){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	
		xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  ����ѧϰ�ɹ��������
  * @param 	next ��������Ԫ�� Ӧָ�򷵻ذ�ͷ��
  * @retval None
  */
void WifiIntStrReport()
{
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntString					=	(ST_IntString *) ( &stp_gWifiDataHeadToSend->next );  //ǿת
	stp_gIntString->result	=	RESULT_TRUE;
	stp_gIntString->strLen	=	st_gRFStudyData.Len+20;//(sizeof(u8)*7); // ���������ǰ��9�����ֺ� len type �����ֽ�
//	portENTER_CRITICAL();
	memcpy((void *)stp_gIntString->content,(void *)&st_gRFStudyData,stp_gIntString->strLen);
//	portEXIT_CRITICAL();
	stp_gIntString->content[stp_gIntString->strLen]		= 0x21;
	stp_gIntString->content[stp_gIntString->strLen+1]	= 0x71;
//	stp_gIntString->strLen += 2;
	st_gWifiReport.reportLen 	= findLen((u8*)st_gWifiReport.reportBuf);
	
	if( st_gWifiReport.reportLen > 10){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	
		xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  Ŀǰ���ڲ�ѯ���ֻ�״̬��ֻ�ظ�����״̬��
  * @param 
  * @retval None
  */
void WifiQueryAllDeviceReport()
{
	u16 len;//,quantity;
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntShortSend	= (ST_IntShort	 *)	( &stp_gWifiDataHeadToSend->next);
	stp_gArr_LCSCC		=(ST_Array_LCSCC *)	( &stp_gIntShortSend->epilog);
	stp_gIntShortSend->result				=RESULT_TRUE;
	stp_gIntShortSend->PROBarArrLen	=	st_gHostInfo.slaveCount+1; //1�������Ӹ����ɸ��ֻ�
	
	stp_gArr_LCSCC->idH							=	(u32)( st_gHostInfo.deviceID >> 32 );
	stp_gArr_LCSCC->idL							=	(u32)st_gHostInfo.deviceID;
	stp_gArr_LCSCC->deviceTpye			=	0x00;
	stp_gArr_LCSCC->vsion1					=	(u8)( st_gHostInfo.version >> 8 );
	stp_gArr_LCSCC->vsion2					=	(u8)st_gHostInfo.version;
	stp_gArr_LCSCC->temp						=	st_gHostInfo.temp;
	stp_gArr_LCSCC->state						=	st_gHostInfo.lockInfo;
	//���ϲ��ְ��������� array ��
	len = st_gHostInfo.slaveCount;
	while(len--){
		stp_gArr_LCSCC=( ST_Array_LCSCC * )( &stp_gArr_LCSCC->next );
		
		stp_gArr_LCSCC->idH					=	0;
		stp_gArr_LCSCC->idL					=	st_gHostInfo.stSlave[len].slaveID;
		stp_gArr_LCSCC->deviceTpye	=	0x01;
		stp_gArr_LCSCC->vsion1			=	(u8)(st_gHostInfo.version>>8);
		stp_gArr_LCSCC->vsion2			=	(u8)st_gHostInfo.version;
		stp_gArr_LCSCC->temp				=	st_gHostInfo.temp;
		stp_gArr_LCSCC->state				=	st_gHostInfo.stSlave[len].slaveState;
	}
	//eiplog
	stp_gArr_LCSCC->next					=	0x21;
	*((&stp_gArr_LCSCC->next)+1)	=	0x71;
	
	
//	stp_gIntShortSend->PROBarArrLen=1;
	
	st_gWifiReport.reportLen			=	findLen( (u8*)st_gWifiReport.reportBuf );
	if( st_gWifiReport.reportLen > 10){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  WifiQuerySwitchReport ��ѯ����״̬
  * @param 	switchID  ��Ҫ��ѯ�Ŀ��ص�ID
  * @retval None
  */
BOOL QuerySwitch(u32 switchID)
{
	u8 i;
	ST_LongCharChar	* volatile st_LCC;
	
	stp_gIntShortSend	= (ST_IntShort		*	)	&stp_gWifiDataHeadToSend->next;
	st_LCC						=	(ST_LongCharChar	*	)(&stp_gIntShortSend->epilog);
	
	stp_gIntShortSend->result 			=	RESULT_TRUE;
	stp_gIntShortSend->PROBarArrLen	=	1; //�̶���1������״̬ ��ѯҲֻ��ѯһ������״̬
	
	for(i = 0; i< SWITCH_LIST_MAX; i++){
		if(st_gHostInfo.stSwitch[i].switchID == switchID)
			break;
	}
	if(i < SWITCH_LIST_MAX){
		st_LCC->slaveIDH						=	0;
		st_LCC->slaveIDL						=	switchID;
		st_LCC->studyModeOrChannel	=	st_gHostInfo.stSwitch[i].channle;
		st_LCC->state								=	st_gHostInfo.stSwitch[i].state;
		
		*((&st_LCC->state	)+1)		=	0x21;
		*((&st_LCC->state	)+2)		=	0x71;
		st_gWifiReport.reportLen	=	findLen((u8*)st_gWifiReport.reportBuf);
		if( st_gWifiReport.reportLen > 10 ){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		} 
		xSemaphoreGive(MutexSemaphor);
		return TRUE;
	}else
		xSemaphoreGive(MutexSemaphor);
		ERROR_REPORT();
		return FALSE;
}
/**
  * @brief  DeviceLogin
  * @param 	next 	ָ��ָ���ͷ���������	
  * @retval None
  */
void DeviceLogin(u8 mode,BaseType_t * xHigherPriorityTaskWoken)
{

	ST_Long * stp_DeviceID;
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_DeviceID = (ST_Long *)&stp_gWifiDataHeadToSend->next;
	
	stp_gWifiDataHeadToSend->head			=	PKT_HDR;
	stp_gWifiDataHeadToSend->version 	= 1;
	stp_gWifiDataHeadToSend->cmd			=	LOG_IN;
	stp_gWifiDataHeadToSend->sn				= 1;
	stp_gWifiDataHeadToSend->len			=	8;
	stp_gWifiDataHeadToSend->reserveL	=	0;
	stp_gWifiDataHeadToSend->reserveH	=	0;
	
	stp_DeviceID->hostID1				=	(u32)(st_gHostInfo.deviceID>>32);
	stp_DeviceID->hostID2				=	(u32)st_gHostInfo.deviceID;
	//epilog
	stp_DeviceID->next					=	0x21;
	*((&stp_DeviceID->next)+1)	=	0x71;
	
	st_gWifiReport.reportLen		=	findLen((u8*)st_gWifiReport.reportBuf);
	
	if( st_gWifiReport.reportLen > 10){
		if(mode==0){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����	
		}else if(mode==1){
//		 xTaskNotifyFromISR(SendWifiTaskHandle,(u32 )len,eSetValueWithOverwrite,xHigherPriorityTaskWoken);
		}			
	} 
	xSemaphoreGive(MutexSemaphor);
}

/**
  * @brief  WifiHeartbeatReport �����ϱ�Ŀǰֻ������״̬  �ܿص�����ͷ�������Դӷ����б���ȡ�ã���
	* @param 	mode: 1 ���жϷ���  0 ����ͨ����
	*					xHigherPriorityTaskWoken: �ж��Ƿ��и������ȼ� ����ͨģʽ�¿�ֱ�Ӵ���NULL
  * @retval None
  */
void WifiHeartbeatReport(u8 mode,BaseType_t * xHigherPriorityTaskWoken)
{

	ST_LongStirng * stp_LongDeviceID;
	xSemaphoreTake(MutexSemaphor,10);
	
//	while(st_gWifiReport.dataLock){
//		vTaskDelay(20);
//	}
//	st_gWifiReport.dataLock = 1;
	
	stp_gWifiDataHeadToSend->head			= PKT_HDR;
	stp_gWifiDataHeadToSend->version	= st_gHostInfo.version;
	stp_gWifiDataHeadToSend->cmd			= 0x0002;
	stp_gWifiDataHeadToSend->sn				= 2; //�����к�
	stp_gWifiDataHeadToSend->len			= 0; //��������ڷ��͵�������������
	stp_LongDeviceID 									= (ST_LongStirng *)&stp_gWifiDataHeadToSend->next;
	
	stp_LongDeviceID->slaveIDH				=	(u32)(st_gHostInfo.deviceID >> 32);
	stp_LongDeviceID->slaveIDL				=	(u32)st_gHostInfo.deviceID;
	stp_LongDeviceID->stringLen=1; //�����ʾ�豸����
	
	stp_gArr_LCSCC										=	(ST_Array_LCSCC *) &stp_LongDeviceID->content;
	
	stp_gArr_LCSCC->idH								=	(u32)(	st_gHostInfo.deviceID	>> 32);
	stp_gArr_LCSCC->idL								=	(u32)st_gHostInfo.deviceID;
	stp_gArr_LCSCC->deviceTpye				=	0;
	stp_gArr_LCSCC->vsion1						=	(u8)(st_gHostInfo.version >> 8);
	stp_gArr_LCSCC->vsion2						=	(u8)st_gHostInfo.version;
	stp_gArr_LCSCC->temp							=	st_gHostInfo.temp;
	stp_gArr_LCSCC->state							=	st_gHostInfo.lockInfo;
	//eiplog
	stp_gArr_LCSCC->next					=	0x21;
	*((&stp_gArr_LCSCC->next)+1)	=	0x71;
	
	st_gWifiReport.reportLen	=	findLen((u8*)st_gWifiReport.reportBuf);
	
	if( st_gWifiReport.reportLen > 10){
		Wifi_QueueIn( &st_gWifiReport );
//		st_gWifiReport.dataLock = 0;
	} 
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  SwitchHostLockState	�л�����״̬���������󲻿ɱ�00��ַ�㲥��
	*					lockInfo=0		����
	*					lockInfo=1		���ߣ�δ������
	*					lockInfo=2		���ߣ���������
  * @param 	None
  * @retval None
  */
void SwitchHostLockState(void)
{
	xSemaphoreTake(MutexSemaphor,10);
	
	if(st_gHostInfo.lockInfo == 1){
		st_gHostInfo.lockInfo 	= 2;
	}else{
		st_gHostInfo.lockInfo 	= 1;
	}
	STMFLASH_Write(LOCK_FLAG_FLASH_ADDR,&st_gHostInfo.lockInfo,1);
	
	stp_gIntSend							= (ST_Int*)&stp_gWifiDataHeadToSend->next;
	stp_gIntSend->result			= RESULT_TRUE;
	stp_gIntSend->epilog			= EPILOG;
	
	st_gWifiReport.reportLen	=	findLen((u8*)st_gWifiReport.reportBuf);
	if( st_gWifiReport.reportLen > 10){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief AddTimTaskList  ��Ӷ�ʱ�����б�
  * @param next ���յ���WIFI����ָ�룬Ӧָ�����ƣ������ƿ�ʼ����
	*					Flash�洢��ʽ����
	*		��		�ܳ���		(short) �������Ƶ� !q �ĳ���(����!q)
	*		|			ͷ������	(short)	�������Ƶ����ڵĳ���(��������)
	*		|			״̬		(short)    //������һ��short�����淽��flash
	*		|			����		(len+content)	
	*		|			Ƶ��		(Byte)
	*					ʱ��		(len+content)
	*					����		(len+content)
	*					��������(len+ content)
	*							--	ID				(long)
	*							--	������			(len+conten)
	*							--	ִ����ʱ		(short)
	*		��		״̬		(Byte)
	*	@param mode	���Ϊ0��ֱ�Ӳ��ҿհ׵ĵط�д��
	*								�����Ϊ0�Ͱ���mode��λ��д��
	*								������λΪ1��ô����ֱ��ɾ��
  * @retval u8 ����ɹ� 
	*									���ص���Ӷ�ʱ���������ID
	*						 ���ʧ��
	*									����0
  */


u8 ModifyTimeTaskList(const u8 * next,u8 mode)
{
	u16 us_lTotalLen=0,us_lHeadLen=0;
//	u32 ul_rtcTimestamp;
	u8 uc_cycleID=0; u8 uc_cycleDay;
	u8 uc_week=0;
	u8 const *p=NULL;
	 //�����ɾ������Ҫ���������κδ���ֱ��ɾ��
	if(mode & 0x80){ //�����λΪ1��ʾɾ��
		memset(&uc_gTimTaskList2Arr[uc_cycleID][0],0,TIMINGTASK_HEAD_LEN_MAX); //�������ͷ���б�
		STMFLASH_Write(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*uc_cycleID),(u16 *)(&uc_gTimTaskList2Arr[uc_cycleID][0]),TIMINGTASK_HEAD_LEN_MAX);
		st_gHostInfo.timeTaskCount--;
		return RESULT_TRUE;
	}
	p=next;
	us_lTotalLen=0;
	us_lHeadLen=0;
	us_lTotalLen=findLen((const u8 *)next);
//	p+=8; //ָ�����Ƶ�string(len)
	us_lHeadLen+=*(u16*)p;																					//ָ��		����len
	p+=*(u16*)p;p+=2;//ָ��Ƶ��byte +*p��Խ������+2��Խ����strLen��2���ֽ�    ָ��			Ƶ��
	p+=1;     //ָ��ʱ���string(len)																//ָ��		ʱ��len
	us_lHeadLen += *(u16*)p;
	p+=*(u16*)p; p+=2; //ָ������string(len)
	us_lHeadLen+=*(u16*)p;
	us_lHeadLen+=(2+1+2+2); // ���Ƴ�short Ƶ��Byte��ʱ�䳤short�����ڳ�short

	if(mode == ADD_LIST_MODE){	//�������Ӿ�ֱ����һ���հ׵ĵط����
	for(uc_cycleID=0;uc_cycleID<TIMINGTASK_LIST_LEN_MAX;uc_cycleID++){
		if(strcmp((const char *)&uc_gTimTaskList2Arr[uc_cycleID][0],"")==0){ //�����ʱ��ʱ�����б��ǿյľʹ洢
				st_gHostInfo.timeTaskCount++;
				break;
			}
		}
		if(uc_cycleID==TIMINGTASK_LIST_LEN_MAX){ //����������������һ��
			uc_cycleID=TIMINGTASK_LIST_LEN_MAX-1;
		}
	}else{			//������޸Ķ�ʱ�������ֱ�Ӱ����·�������ID���޸ģ����ڿͻ��˲���ʶ��Ϊ0��ID����ID��+1
		uc_cycleID = ( mode & ADD_LIST_MODE);
		if(uc_cycleID > TIMINGTASK_LIST_LEN_MAX)
			return RESULT_FALSE;
	}
	uc_gTimTaskList2Arr[uc_cycleID][0]=(u8)us_lHeadLen;
	uc_gTimTaskList2Arr[uc_cycleID][1]=(u8)(us_lHeadLen>>8); //�ͷŵͣ��߷Ÿ�
	memcpy(&uc_gTimTaskList2Arr[uc_cycleID][2],next,us_lHeadLen); //��ͷ���� ���� ���� Ƶ�� ʱ�� ���� ״̬���浽����
	uc_gTimTaskList2Arr[uc_cycleID][us_lHeadLen+2]=*(next+us_lTotalLen-3); //��״̬Ҳ�洢��ͷ������ (״̬�����ݵ��������0x7121ǰ������us_lTotalLenҪ-3ָ��!q֮ǰ��һ���ֽ�)
																										//us_lHeadLen+2���ƶ�2λ�����ȡ���ռ�Ŀռ�
	p=next;
	/**�Ѷ�ʱ���������е�ͷ���ϳ��Ⱥ�״̬����**/
	*((u16 *)(p-2))=uc_gTimTaskList2Arr[uc_cycleID][us_lHeadLen+2];//��״̬ 
	*((u16 *)(p-4))=us_lHeadLen;		//��ͷ������
	*((u16 *)(p-6))=us_lTotalLen; //�ɵ͵������ܳ���
 
	us_lTotalLen+=6; //���������5���ֽ�
	us_lTotalLen=(us_lTotalLen+(us_lTotalLen%2))/2; //ֻ���԰���д������/2������ǵ���Ҫ��1��˫����/2
	
	STMFLASH_Write(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*uc_cycleID),(u16 *)(next-6),us_lTotalLen);
	
	p=next;
	p+=*(u16*)p;p+=2;	//ָ��Ƶ��byte +*p��Խ��str����+2��Խ����strLen��2���ֽ�    ָ��			Ƶ��
	p+=1;	     	//ָ��ʱ���string(len)	
//	p+=2;			 	//ָ��ʱ��
	
	if(uc_gTimTaskList2Arr[uc_cycleID][us_lHeadLen+2] ==1 ){  //����������˲���Ҫ����ʱ�����
		//�����ݰ���ȡ��ʱ��value
		Extract_Time(&st_gRTCTime,p);
		p-=1; //���Ƶ�ָ��Ƶ��
		if(*p &0x80){ //*(p-2)ָ��Ƶ��
			HAL_RTC_GetTimeUser(&hrtc);  //ȡ��ʱ�任���������
			uc_week=hrtc.DateToUpdate.WeekDay;
			//ѭ��7��鿴������Ҫִ��
			for(uc_cycleDay=0;uc_cycleDay<7;uc_cycleDay++)
			{ 
				if(uc_week > 7) uc_week = 1; //����������������ֱ�Ӹ�ֵ������һ
				if(*p & (1<<(uc_week-1))){ //�����Ӧ������ִ�� //����WeekDay-1������ǰ7λ��ʾ���ڼ�����1�Ͳ���������
					
					
					//������������ý�������ڸ���
					st_gRTCTime.date.Date		=	hrtc.DateToUpdate.Date + uc_cycleDay; //ִ��ʱ�������������������ȥ
					st_gRTCTime.date.Month	=	hrtc.DateToUpdate.Month;
					st_gRTCTime.date.Year		=	hrtc.DateToUpdate.Year;
					//�������Ӧ��ʱ���
					ul_gTimeTaskUnixTimArr[uc_cycleID]=RTC_ToUTC(&st_gRTCTime);
					break;
				}
				uc_week++; //���ڼ���һ�� �����ǲ���������Ҫִ��
			}
		}else{
			//��ʱ��Vauleת����Unix Timstamp;
			ul_gTimeTaskUnixTimArr[uc_cycleID]=RTC_ToUTC(&st_gRTCTime);
		}
		CalcMinTimstamp(); //�ҳ������Ҫִ�е����������5����֮�ھ�������
	}
	return (uc_cycleID|0x80);
}
/**
  * @brief  ModifyTimeTaskSate
  * @param 	id 		����ID
	*					state	�����Ŀ��״̬
  * @retval None
  */
void ModifyTimeTaskSate(u32 id,u8 state)
{
	u16 us_lLen;
	if(id>TIMINGTASK_LIST_LEN_MAX){
		ERROR_REPORT();
	}else{
		//ȡ��ͷ������
		us_lLen=uc_gTimTaskList2Arr[id][1];//�ͷŵͣ��߷Ÿ�
		us_lLen<<=8;
		us_lLen|=uc_gTimTaskList2Arr[id][0];
		uc_gTimTaskList2Arr[id][us_lLen+2]=state;
		
		STMFLASH_Write(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*id)+4,(u16*)&state,1);
		
		if(state == 0){  //������ɿ������ر�
			if(ul_gTimeTaskUnixTimArr[id] == RECENT_TASK_UTC){//�����ǰ������������Ҫִ�е�����
				ul_gTimeTaskUnixTimArr[id] = 0;
				CalcMinTimstamp(); //
			}
			ul_gTimeTaskUnixTimArr[id] = 0;
		}else if(state == 1){ //��0�䵽1��Ҫ���㲢�������������������
			//ȡ��strLen
			memcpy(&us_lLen,&uc_gTimTaskList2Arr[id][2],2);  //ȡ�����Ƶ�string len
			//�����ӦID��ʱ���
			if((uc_gTimTaskList2Arr[id][us_lLen + 4] & 0x80) == 0){
				CalcSpecificNormalTask(id);
			
			}else{
				CalcSpecificPeriodicTask(id,TRUE);
			}
			//�ҳ������Ҫִ�е�����
			CalcMinTimstamp(); //
		}
		SUCCESS_REPORT();
	}
}
/**
  * @brief GetTimeTaskList  ��ȡ��ʱ�����б�
  * @param  startPos ��ʼλ��
	*					quantity һ�λ�ȡ���ٸ������б�
  * @retval None
  */
void GetTimeTaskList(u32 startPos,u32 quantity)
{
	u8 i,* volatile p;
	u16 us_lLen=0;
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntIntSend=(ST_IntIntS *)&stp_gWifiDataHeadToSend->next;
	stp_gIntIntSend->result=RESULT_TRUE;
	stp_gIntIntSend->taskID=st_gHostInfo.timeTaskCount; //�����->taskID��ʽ��������������
	stp_gIntIntSend->epilog=(u16)st_gHostInfo.timeTaskCount; //�����->epilog��ʽ�����������б�Array Len
	
	p=(u8 *)(&stp_gIntIntSend->epilog);
	p+=2;     //ָ��ָ�������б��ȣ����鳤�ȣ��ֶκ��2 Խ��
	for(i=0;i<TIMINGTASK_LIST_LEN_MAX;i++){
		//�ѱ�������ͷ����ӽ�ȥ
		if(strcmp((const char *)&uc_gTimTaskList2Arr[i][0],"")!=0){
			*(u32*)p=i+1;//��������ID(�ͻ��˲���ʶ��0��ID��������ID��1)
			p+=4;			 //ָ���ƶ�������ID��
			//��ȡ��ͷ������
			us_lLen=uc_gTimTaskList2Arr[i][1];//�ͷŵͣ��߷Ÿ�
			us_lLen<<=8;
			us_lLen|=uc_gTimTaskList2Arr[i][0];
			memcpy(p,&uc_gTimTaskList2Arr[i][2],us_lLen+1);//ͷ������û�м���״̬ ����+1
			p+=us_lLen+1;//ָ���ƶ�
		}
	}
	*(u16*)p=EPILOG; //�ѽ�������ӽ�ȥ
	
	st_gWifiReport.reportLen = findLen((u8*)st_gWifiReport.reportBuf);
	if( st_gWifiReport.reportLen > 10){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	xSemaphoreGive(MutexSemaphor);
}	
/**
  * @brief  Get_TimeTaskInfo ��ȡ������ʱ������Ϣ
  * @param 	taskID	����Ҫ��ȡ������ID
  * @retval None
  */
void Get_TimeTaskInfo(u32 taskID)
{
	u16 taskLen,*p;
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntIntSend=(ST_IntIntS *)&stp_gWifiDataHeadToSend->next;
	stp_gIntIntSend->result	=	RESULT_TRUE;
	stp_gIntIntSend->taskID	=	taskID+1;
	
	p	=	(u16 *)&stp_gIntIntSend->taskID;
	p	+=	2;         //ָ��ָ��taskID���ƶ�4���ֽ�
	
	if( taskID > TIMINGTASK_LIST_LEN_MAX){
			ERROR_REPORT();
	}else{
		taskLen	=	STMFLASH_ReadHalfWord( TIME_LIST_FLASH_ADDR + ( TIMINGTASK_DATALEN_MAX * taskID ) ); //��ȡ���ܳ���
		
		STMFLASH_Read( TIME_LIST_FLASH_ADDR + ( TIMINGTASK_DATALEN_MAX * taskID) + 6, p, ( taskLen + taskLen % 2 ) /2 ); //�������ݵ�st_gWifiReport.reportBuf
		
		st_gWifiReport.reportLen = findLen((u8*)st_gWifiReport.reportBuf);
		
		//��ȡ״̬��ĩβ
		st_gWifiReport.reportBuf[ st_gWifiReport.reportLen - 3 ] = (u8)(STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*taskID)+4));
		
		if( st_gWifiReport.reportLen > 10 ){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi����						
		}
	}
	xSemaphoreGive(MutexSemaphor);
}

/**
  * @brief  ExecuteTask ִ�ж�ʱ���� ֻ��Ҫ����ID ��������ID����������ִ�������� 
  * @param 	taskID ������Ҫִ�е�ID
  * @retval TRUE  ִ�гɹ�
	*					FALSE	ִ��ʧ��
  */

BOOL ExecuteTask(u8 taskID)
{
	u16 		us_headLen 		= 0;
	u16 		us_totalLen 	= 0;
	u16 		us_cmdArrLen 	= 0;
	u16 		us_cmdLen 		= 0;
	u8			*p						=NULL;
	uint64_t	ull_addr 		=	0;
	//����ID������ĳ��ȶ�����
	us_totalLen =	STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * taskID));
	us_headLen 	= STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * taskID) + 2);
	if((us_totalLen > TIMINGTASK_DATALEN_MAX) || (us_headLen > TIMINGTASK_HEAD_LEN_MAX)){
		return FALSE;
	}
	//�����������鶼������ 
	STMFLASH_Read(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*taskID)+6,(u16 *)uc_gTimCmdArr,(us_totalLen+us_totalLen%2)/2);
	//ȡ��ִ����������ĳ��ȣ���
	//	us_cmdArrLen = uc_gTimCmdArr[us_headLen]; //ȡ��λҲ������Ϊ�������鲻���ܴ���255  ȡһ��16λ�ĸ���ȫ
	memcpy((void *)&us_cmdArrLen,&uc_gTimCmdArr[us_headLen],2);
	//ָ��ָ�� "ִ���豸ID" +2 Խ�����������鳤�ȡ�
	p = &uc_gTimCmdArr[us_headLen+2];
	
	//����ֻΪ���񿪱���һ�� TIMINGTASK_DATALEN_MAX ���ֽڵĿռ�  ��������ô��� TIMINGTASK_CMD_MAX
	if(us_cmdArrLen <= TIMINGTASK_CMD_MAX){
		while(us_cmdArrLen){
			
			memcpy(&ull_addr,p,8);
			//+8Խ��ִ���豸ID
			p += 8;
			//���Ƴ�16λ�� �����볤��
			memcpy((void *)&us_cmdLen,p,2);
			//Խ�� �����볤��  ָ����������
			p	+= 2;
			
			//����豸��ַ�ǲ��Ǳ���ַ (����ַ��ֱ��ִ��)
			if(st_gHostInfo.deviceID == ull_addr){
				//ִ������
				CONTROL_FUN(p,us_cmdLen);
			
			}else{ //�Ǳ���ַ�ͷ��͵���Ӧ��ַ
				
				CONTROL_SLAVE_FUN((u32)ull_addr, us_cmdLen, p);
			}
			p	+= us_cmdLen;		//Խ�������������ݡ�
			p += 2;						//Խ����ִ����ʱ��
			us_cmdArrLen--;
			
		}
		return TRUE;
	}else{
		//_Error_Handler(__FILE__, __LINE__);
		return FALSE;
	}
}
/**
  * @brief  DataInit	��ʼ������ ����״̬�����ݣ���ʱ�������ݣ��ֻ��Ϳ��� ��ַ����
  * @param 
  * @retval None
  */
void DataInit(void)
{
	u16 us_taskHeadLen;
	u16 us_tasktotalLen;
	u8 	i;
	//��ȡst_gHostInfo����Ҫ��ȡ������
//	st_gHostInfo.deviceID=
	//��ȡ����״̬ �����Ƿ��������������󲻿ɱ�00��ַ�㲥��
	st_gHostInfo.lockInfo 		= (u8) STMFLASH_ReadHalfWord(LOCK_FLAG_FLASH_ADDR);
	st_gHostInfo.version			=	VER;
	st_gHostInfo.slaveCount		=	0;
	st_gHostInfo.switchCount	=	0;
	st_gHostInfo.timeTaskCount=	0;
	if((st_gHostInfo.lockInfo == 0xFF) || st_gHostInfo.lockInfo == 0)  st_gHostInfo.lockInfo = 0x01;
	//ȡ�������豸��ַ 64λ
//	st_gHostInfo.deviceID			= STMFLASH_ReadHalfWord(DEVIDH_FLASH_ADDR + 2); //ȡ������оƬ��UID��32λ
//	st_gHostInfo.deviceID		<<=	16; //����16λ
//	st_gHostInfo.deviceID		 |=	STMFLASH_ReadHalfWord(DEVIDH_FLASH_ADDR);
//	st_gHostInfo.deviceID		<<=	16; //����16λ
//	st_gHostInfo.deviceID		 |=	STMFLASH_ReadHalfWord(ROLLING_CODE_ADDR + 2); //ȡ���������4���ֽ�
//	st_gHostInfo.deviceID		<<=	16; //����16λ
//	st_gHostInfo.deviceID		 |=	STMFLASH_ReadHalfWord(ROLLING_CODE_ADDR);
		st_gHostInfo.deviceID = ((uint64_t)((*(__IO u32*)DEVIDL_FLASH_ADDR)|(*(__IO u32*)DEVIDH_FLASH_ADDR)) << 32) |((*(__IO u32*)DEVIDM_FLASH_ADDR)|(*(__IO u32*)DEVIDH_FLASH_ADDR)); 
	//��ȡ����ӹ��ķֻ�������
	for(i = 0;i < SLAVE_LIST_MAX;i++){
		STMFLASH_Read(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),	(u16 *)( & st_gHostInfo.stSlave[i].slaveID),	2);
		if(st_gHostInfo.stSlave[i].slaveID == 0xFFFFFFFF){
			st_gHostInfo.stSlave[i].slaveID	= 0;
		}else if(st_gHostInfo.stSlave[i].slaveID == 0){
			;//Do nothing
		}else{
			st_gHostInfo.stSlave[i].slaveState = 1;
			st_gHostInfo.slaveCount++;  //�Ȳ�����0Ҳ��ȫ��FF˵������������ݵġ�+1
		}
	}
	//��ȡ����ӹ��Ŀ��ز�����
	for(i = 0;i < SWITCH_LIST_MAX;i++){
		STMFLASH_Read(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i),	(u16 *)( & st_gHostInfo.stSwitch[i].switchID),	2);
		if(st_gHostInfo.stSwitch[i].switchID == 0xFFFFFFFF){
			st_gHostInfo.stSwitch[i].switchID	= 0;
		}else if(st_gHostInfo.stSwitch[i].switchID == 0){
			;//Do nothing
		}else{
			//�������ص�λ�����Ҽ�����1
			st_gHostInfo.stSwitch[i].channle = (u8)STMFLASH_ReadHalfWord(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i) + 4);
			st_gHostInfo.switchCount++;  //�Ȳ�����0Ҳ��ȫ��FF˵������������ݵġ�+1
		}
	}
	//��ȡ��ʱ��������
	for(i = 0; i< TIMINGTASK_LIST_LEN_MAX; i++){
		//ȡ��ͷ�����Ⱥ��ܳ���
		us_tasktotalLen	=	STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * i));
		us_taskHeadLen 	= STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * i) + 2);
		
		if((us_tasktotalLen == 0xFFFF) || (us_taskHeadLen == 0xFFFF)){
			us_tasktotalLen = 0;
			us_taskHeadLen	=	0;
		}
		//��������б���FLASH�������ݾͶ��������б�������
		if(us_taskHeadLen != 0){
			memcpy(&uc_gTimTaskList2Arr[i][0], &us_taskHeadLen, 2);//��2���ֽڵ�ͷ�����ȷŵ������б�������ȥ
					//(us_taskHeadLen + us_taskHeadLen%2)/2  ȡ����
			STMFLASH_Read(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * i)+6,
										(u16*)&uc_gTimTaskList2Arr[i][2],
										(us_taskHeadLen + us_taskHeadLen % 2) / 2
									 );
			uc_gTimTaskList2Arr[i][us_taskHeadLen + 2] = (u8)STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * i) + 4);
			st_gHostInfo.timeTaskCount++;
		}
	}
}
/**
  * @brief  EraseDevice ����WIFIģ��������Ϣ���豸Flash
  * @param 
  * @retval None
  */
void EraseDevice(void)
{
	u8 	i;
	//�������״̬
	st_gHostInfo.lockInfo 		= 1; //1�������� 0��ʾ������ 2��ʾ����
	STMFLASH_Erase_Word(LOCK_FLAG_FLASH_ADDR);
	
	st_gHostInfo.version			=	VER;
	st_gHostInfo.slaveCount		=	0;
	st_gHostInfo.switchCount	=	0;
	st_gHostInfo.timeTaskCount=	0;
	
	for(i = 0;i < SLAVE_LIST_MAX;i++){
		//����������� �����Ϊ��
		if(st_gHostInfo.stSwitch[i].switchID != 0){
			st_gHostInfo.stSwitch[i].switchID = 0;
			st_gHostInfo.stSwitch[i].channle	=	0;
			STMFLASH_Erase_Word(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i));
		}
		//����ֻ����� �����Ϊ��
		if(st_gHostInfo.stSlave[i].slaveID != 0){
			st_gHostInfo.stSlave[i].slaveID 		= 0;
			st_gHostInfo.stSlave[i].slaveState	= 0;
			STMFLASH_Erase_Word(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i));
		}
		if(memcmp(&uc_gTimTaskList2Arr[i][0],"",4) != 0 ){
			memset(&uc_gTimTaskList2Arr[i][0],0,TIMINGTASK_HEAD_LEN_MAX);
			STMFLASH_Erase_Word(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * i));
		}
	}
}
