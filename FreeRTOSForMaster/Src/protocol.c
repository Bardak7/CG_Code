#include "protocol.h"
#include "string.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "stmflash.h"
#include <stdlib.h>
#include "rtc.h"

ST_HostInfo	st_gHostInfo;

extern  const volatile ST_RFIRStudy 	 st_gRFStudyData; //这个结构体在本文件中是不可更改的。

static ST_Report st_gWifiReport;

void Extract_Time(ST_Time* time,const u8 *timePos);

u16 findLen(const u8 * source );

						/**以下结构体用于WIFI格式化发送的数据**/
ST_WifiDataPackHead		*const	stp_gWifiDataHeadToSend = (ST_WifiDataPackHead 	*)st_gWifiReport.reportBuf; //包头的指针永远只指向发送数组的第一个位置 const其指针其指针所指不可变
ST_IntIntS			*	volatile			stp_gIntIntSend;//			=	(ST_IntIntS		*)&stp_gWifiDataHeadToSend->next;//在定义时强制转换必须是一个const类型
ST_Int					*	volatile			stp_gIntSend;
ST_IntChar			*	volatile			stp_gIntCharSend;
ST_IntShort			*	volatile			stp_gIntShortSend;
ST_IntLong			*	volatile			stp_gIntLongSend;
ST_IntLongCC		*	volatile			stp_gIntLongCCSend;
ST_IntString		*	volatile			stp_gIntString;
ST_Array_LCSCC	*	volatile			stp_gArr_LCSCC; //

/**
	* 任务数组只存储任务头部  内容如下
	*  			头部长度(short)  这里的头部长度只代表“头部数据”的长度，不包含头部本身的short和任务状态的Byte
	*				头部数据(复合结构) 名称 频次 时间 日期 
	*				任务状态(Byte)
**/
u8 uc_gTimTaskList2Arr[TIMINGTASK_LIST_LEN_MAX][TIMINGTASK_HEAD_LEN_MAX]; //
/*作为定时任务从Flash中取出的命令缓存*/
u8 uc_gTimCmdArr[TIMINGTASK_DATALEN_MAX];
/**
	* 保存任务执行时间对应的时间戳，如果没有这个任务或没有使能就为0 下标与ID 对应 +1 最后的那个是为了保存最近值
**/
u32 ul_gTimeTaskUnixTimArr[TIMINGTASK_LIST_LEN_MAX+1]; //保存定时任务转换后的unix时间

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
  * @brief  Extract_Time 把客户端的字符的时间转换成数值
  * @param 	time ：ST_Time类型的结构体指针 保存转换后的数值日期
	*					
	*					timePos：时间开始位置
  * @retval None
  */
void Extract_Time(ST_Time* time,const u8 *timePos)//u8 * datePos,
{
  u16 us_timeLen;
	us_timeLen = *(u16 *)timePos;
	timePos += 2; //越过
	time->Time.Hours			=	(u8)(atoi((const char*)timePos));					//指针指向具体时间的小时
	time->Time.Minutes		=	(u8)(atoi((const char*)(timePos+=3)));  	//指针指向具体时间的分
	if(us_timeLen == 5){
		time->Time.Seconds	=	0;
	}else{
		time->Time.Seconds	=	(u8)(atoi((const char*)(timePos+=3)));
	}
	timePos+=2;  	//指向日期String(len)
	timePos+=2;		//指向日期的年
	time->date.Year=atoi((const char*)timePos);
	time->date.Month=(u8)atoi((const char*)(timePos+=5));		//指向日期的月
	time->date.Date=(u8)atoi((const char*)(timePos+=3));		//指向日期的日
	
}
/**
  * @brief  findLen 从接收的数据中 从给定的位置找到最近的 0x21 0x71结束符
  * @param 	source	需要查找的起始地址
  * @retval u16 		从起始地址到 0x7121的长度，包含0x7121;
  */
u16 findLen(const u8 * source )
{
  u16 counter=0;
	while(1){
		
		counter++;
//	  if(* source==0x71 ){//&& (*source--)==0x21){ //&&左结合，如果＝＝0x71后再计算右边
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

/*从数据体里格式化出 64位的数据*/
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
 * Description  : 向ESP8266获取sntp时间 此函数不可以中断中被调用
 * Parameters   :	none
 * Returns      : 返回 RESULT_FALSE入队失败。RESULT_TRUE 成功
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
 * Description  : 向ESP8266获取连网状态 此函数不可在中断中被调用
 * Parameters   :	none
 * Returns      : 返回 RESULT_FALSE入队失败。RESULT_TRUE 成功
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
  * @brief 	WifiIntLongReport 返回给客户端 INT +LONG
  * @param 	next 	指针指向包头后面的数据	
	*					idH 高4位  idL 低4位
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
  * @brief  返回成功或失败的给客户端，不带其它状态等。
  * @param next 	指针指向包头后面的数据	
	*				 result	需要返回给客户端的结果	RESULT_TRUE：成功 RESULT_FALSE： 失败 其它返回不用带结果回复
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
		Wifi_QueueIn( &st_gWifiReport );//Wifi发送						
	}
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  WifiIntIntReport
  * @param 	
	*					id 		任务ID
  * @retval None
  */
void WifiIntIntReport(u32 id)
{
	xSemaphoreTake(MutexSemaphor,10);
	stp_gIntIntSend						=	(ST_IntIntS *)( &stp_gWifiDataHeadToSend->next );
	stp_gIntIntSend->result		=	RESULT_TRUE; //如果调用这个函数一定是已成功执行了
	stp_gIntIntSend->taskID		=	id;
	stp_gIntIntSend->epilog		=	EPILOG; 		
	st_gWifiReport.reportLen	=	findLen( (u8*)st_gWifiReport.reportBuf );
	
	if( st_gWifiReport.reportLen > 10 ){
		Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  返回给客户端一个4个字节的result和1个字节的状态   控制开关	控制设备	设备锁定	
  * @param 	next 	指针指向包头后面的数据	
	*					state 需要返回的状态
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
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	xSemaphoreGive(MutexSemaphor);
}

/**
  * @brief  返回添加开关数据  由于添加开关和分机
  * @param 	next 接下来的元素
	*					addr 添加开关和分机只需要一个32位的地址而不需要64位地址，高位
	*					channel 通道，
	*					state	状态
  * @retval None
  */
void WifiIntLongCCReport(u32 addr,u8 channel,u8 state)
{
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntLongCCSend					=	(ST_IntLongCC *)(	&stp_gWifiDataHeadToSend->next	);
	stp_gIntLongCCSend->result	=	RESULT_TRUE;
	stp_gIntLongCCSend->idH			=	0;
	stp_gIntLongCCSend->idL			=	addr; //这里做了一个64位的小端转换 idH 和 idL对换了一下。
	stp_gIntLongCCSend->channle	=	channel;
	stp_gIntLongCCSend->state		=	state;
	stp_gIntLongCCSend->epilog	=	EPILOG;
	st_gWifiReport.reportLen		=	findLen((u8*)st_gWifiReport.reportBuf);
	
	if( st_gWifiReport.reportLen > 10 ){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	
		xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  返回学习成功后的数据
  * @param 	next 接下来的元素 应指向返回包头后
  * @retval None
  */
void WifiIntStrReport()
{
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntString					=	(ST_IntString *) ( &stp_gWifiDataHeadToSend->next );  //强转
	stp_gIntString->result	=	RESULT_TRUE;
	stp_gIntString->strLen	=	st_gRFStudyData.Len+20;//(sizeof(u8)*7); // 数据体加上前面9个半字和 len type 两个字节
//	portENTER_CRITICAL();
	memcpy((void *)stp_gIntString->content,(void *)&st_gRFStudyData,stp_gIntString->strLen);
//	portEXIT_CRITICAL();
	stp_gIntString->content[stp_gIntString->strLen]		= 0x21;
	stp_gIntString->content[stp_gIntString->strLen+1]	= 0x71;
//	stp_gIntString->strLen += 2;
	st_gWifiReport.reportLen 	= findLen((u8*)st_gWifiReport.reportBuf);
	
	if( st_gWifiReport.reportLen > 10){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	
		xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  目前用于查询主分机状态，只回复主机状态。
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
	stp_gIntShortSend->PROBarArrLen	=	st_gHostInfo.slaveCount+1; //1个主机加个若干个分机
	
	stp_gArr_LCSCC->idH							=	(u32)( st_gHostInfo.deviceID >> 32 );
	stp_gArr_LCSCC->idL							=	(u32)st_gHostInfo.deviceID;
	stp_gArr_LCSCC->deviceTpye			=	0x00;
	stp_gArr_LCSCC->vsion1					=	(u8)( st_gHostInfo.version >> 8 );
	stp_gArr_LCSCC->vsion2					=	(u8)st_gHostInfo.version;
	stp_gArr_LCSCC->temp						=	st_gHostInfo.temp;
	stp_gArr_LCSCC->state						=	st_gHostInfo.lockInfo;
	//以上部分把主机填入 array 中
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
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief  WifiQuerySwitchReport 查询开关状态
  * @param 	switchID  需要查询的开关的ID
  * @retval None
  */
BOOL QuerySwitch(u32 switchID)
{
	u8 i;
	ST_LongCharChar	* volatile st_LCC;
	
	stp_gIntShortSend	= (ST_IntShort		*	)	&stp_gWifiDataHeadToSend->next;
	st_LCC						=	(ST_LongCharChar	*	)(&stp_gIntShortSend->epilog);
	
	stp_gIntShortSend->result 			=	RESULT_TRUE;
	stp_gIntShortSend->PROBarArrLen	=	1; //固定回1个开关状态 查询也只查询一个开关状态
	
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
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
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
  * @param 	next 	指针指向包头后面的数据	
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
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送	
		}else if(mode==1){
//		 xTaskNotifyFromISR(SendWifiTaskHandle,(u32 )len,eSetValueWithOverwrite,xHigherPriorityTaskWoken);
		}			
	} 
	xSemaphoreGive(MutexSemaphor);
}

/**
  * @brief  WifiHeartbeatReport 心跳上报目前只报主机状态  受控的命令头部都可以从发送列表中取得，但
	* @param 	mode: 1 从中断发出  0 从普通发出
	*					xHigherPriorityTaskWoken: 判断是否有更高优先级 在普通模式下可直接传入NULL
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
	stp_gWifiDataHeadToSend->sn				= 2; //包序列号
	stp_gWifiDataHeadToSend->len			= 0; //长度域会在发送的任务里计算出来
	stp_LongDeviceID 									= (ST_LongStirng *)&stp_gWifiDataHeadToSend->next;
	
	stp_LongDeviceID->slaveIDH				=	(u32)(st_gHostInfo.deviceID >> 32);
	stp_LongDeviceID->slaveIDL				=	(u32)st_gHostInfo.deviceID;
	stp_LongDeviceID->stringLen=1; //这里表示设备数组
	
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
  * @brief  SwitchHostLockState	切换锁定状态，（锁定后不可被00地址广播）
	*					lockInfo=0		离线
	*					lockInfo=1		在线（未锁定）
	*					lockInfo=2		在线（已锁定）
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
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	
	xSemaphoreGive(MutexSemaphor);
}
/**
  * @brief AddTimTaskList  添加定时任务列表
  * @param next 接收到的WIFI数据指针，应指向名称！从名称开始计算
	*					Flash存储方式如下
	*		低		总长度		(short) ：从名称到 !q 的长度(包含!q)
	*		|			头部长度	(short)	：从名称到日期的长度(包含日期)
	*		|			状态		(short)    //这里以一个short来保存方便flash
	*		|			名称		(len+content)	
	*		|			频次		(Byte)
	*					时间		(len+content)
	*					日期		(len+content)
	*					命令数组(len+ content)
	*							--	ID				(long)
	*							--	命令码			(len+conten)
	*							--	执行延时		(short)
	*		高		状态		(Byte)
	*	@param mode	如果为0就直接查找空白的地方写入
	*								如果不为0就按照mode的位置写入
	*								如果最高位为1那么就是直接删除
  * @retval u8 如果成功 
	*									返回的添加定时任务的任务ID
	*						 如果失败
	*									返回0
  */


u8 ModifyTimeTaskList(const u8 * next,u8 mode)
{
	u16 us_lTotalLen=0,us_lHeadLen=0;
//	u32 ul_rtcTimestamp;
	u8 uc_cycleID=0; u8 uc_cycleDay;
	u8 uc_week=0;
	u8 const *p=NULL;
	 //如果是删除不需要对数据做任何处理直接删除
	if(mode & 0x80){ //如果高位为1表示删除
		memset(&uc_gTimTaskList2Arr[uc_cycleID][0],0,TIMINGTASK_HEAD_LEN_MAX); //清空任务头部列表
		STMFLASH_Write(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*uc_cycleID),(u16 *)(&uc_gTimTaskList2Arr[uc_cycleID][0]),TIMINGTASK_HEAD_LEN_MAX);
		st_gHostInfo.timeTaskCount--;
		return RESULT_TRUE;
	}
	p=next;
	us_lTotalLen=0;
	us_lHeadLen=0;
	us_lTotalLen=findLen((const u8 *)next);
//	p+=8; //指向名称的string(len)
	us_lHeadLen+=*(u16*)p;																					//指向		名称len
	p+=*(u16*)p;p+=2;//指向频次byte +*p是越过内容+2是越过了strLen的2个字节    指向			频次
	p+=1;     //指向时间的string(len)																//指向		时间len
	us_lHeadLen += *(u16*)p;
	p+=*(u16*)p; p+=2; //指向日期string(len)
	us_lHeadLen+=*(u16*)p;
	us_lHeadLen+=(2+1+2+2); // 名称长short 频次Byte，时间长short，日期长short

	if(mode == ADD_LIST_MODE){	//如果是添加就直接找一个空白的地方添加
	for(uc_cycleID=0;uc_cycleID<TIMINGTASK_LIST_LEN_MAX;uc_cycleID++){
		if(strcmp((const char *)&uc_gTimTaskList2Arr[uc_cycleID][0],"")==0){ //如果定时定时任务列表是空的就存储
				st_gHostInfo.timeTaskCount++;
				break;
			}
		}
		if(uc_cycleID==TIMINGTASK_LIST_LEN_MAX){ //如果已添满覆盖最后一个
			uc_cycleID=TIMINGTASK_LIST_LEN_MAX-1;
		}
	}else{			//如果是修改定时任务就是直接按照下发的任务ID来修改，由于客户端并不识别为0的ID所以ID都+1
		uc_cycleID = ( mode & ADD_LIST_MODE);
		if(uc_cycleID > TIMINGTASK_LIST_LEN_MAX)
			return RESULT_FALSE;
	}
	uc_gTimTaskList2Arr[uc_cycleID][0]=(u8)us_lHeadLen;
	uc_gTimTaskList2Arr[uc_cycleID][1]=(u8)(us_lHeadLen>>8); //低放低，高放高
	memcpy(&uc_gTimTaskList2Arr[uc_cycleID][2],next,us_lHeadLen); //把头部的 长度 名称 频次 时间 日期 状态保存到数组
	uc_gTimTaskList2Arr[uc_cycleID][us_lHeadLen+2]=*(next+us_lTotalLen-3); //把状态也存储到头部数据 (状态在数据的最后面在0x7121前面所以us_lTotalLen要-3指向!q之前的一个字节)
																										//us_lHeadLen+2是移动2位“长度”所占的空间
	p=next;
	/**把定时任务数组中的头加上长度和状态加入**/
	*((u16 *)(p-2))=uc_gTimTaskList2Arr[uc_cycleID][us_lHeadLen+2];//再状态 
	*((u16 *)(p-4))=us_lHeadLen;		//再头部长度
	*((u16 *)(p-6))=us_lTotalLen; //由低到高先总长度
 
	us_lTotalLen+=6; //加上上面的5个字节
	us_lTotalLen=(us_lTotalLen+(us_lTotalLen%2))/2; //只能以半字写入所以/2且如果是单数要加1成双数再/2
	
	STMFLASH_Write(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*uc_cycleID),(u16 *)(next-6),us_lTotalLen);
	
	p=next;
	p+=*(u16*)p;p+=2;	//指向频次byte +*p是越过str内容+2是越过了strLen的2个字节    指向			频次
	p+=1;	     	//指向时间的string(len)	
//	p+=2;			 	//指向时间
	
	if(uc_gTimTaskList2Arr[uc_cycleID][us_lHeadLen+2] ==1 ){  //如果任务开启了才需要计算时间戳等
		//从数据包中取出时间value
		Extract_Time(&st_gRTCTime,p);
		p-=1; //回移到指向频次
		if(*p &0x80){ //*(p-2)指向频次
			HAL_RTC_GetTimeUser(&hrtc);  //取出时间换算成年月日
			uc_week=hrtc.DateToUpdate.WeekDay;
			//循环7天查看哪天需要执行
			for(uc_cycleDay=0;uc_cycleDay<7;uc_cycleDay++)
			{ 
				if(uc_week > 7) uc_week = 1; //如果超过了星期天就直接赋值到星期一
				if(*p & (1<<(uc_week-1))){ //如果对应的这天执行 //这里WeekDay-1是由于前7位表示星期几星期1就不用左移了
					
					
					//把任务的日期用今天的日期更换
					st_gRTCTime.date.Date		=	hrtc.DateToUpdate.Date + uc_cycleDay; //执行时间与今天相差的天数加上去
					st_gRTCTime.date.Month	=	hrtc.DateToUpdate.Month;
					st_gRTCTime.date.Year		=	hrtc.DateToUpdate.Year;
					//计算出对应的时间戳
					ul_gTimeTaskUnixTimArr[uc_cycleID]=RTC_ToUTC(&st_gRTCTime);
					break;
				}
				uc_week++; //星期几加一天 看看是不是明天需要执行
			}
		}else{
			//把时间Vaule转换成Unix Timstamp;
			ul_gTimeTaskUnixTimArr[uc_cycleID]=RTC_ToUTC(&st_gRTCTime);
		}
		CalcMinTimstamp(); //找出最近需要执行的任务如果在5分钟之内就设闹钟
	}
	return (uc_cycleID|0x80);
}
/**
  * @brief  ModifyTimeTaskSate
  * @param 	id 		任务ID
	*					state	任务的目标状态
  * @retval None
  */
void ModifyTimeTaskSate(u32 id,u8 state)
{
	u16 us_lLen;
	if(id>TIMINGTASK_LIST_LEN_MAX){
		ERROR_REPORT();
	}else{
		//取出头部长度
		us_lLen=uc_gTimTaskList2Arr[id][1];//低放低，高放高
		us_lLen<<=8;
		us_lLen|=uc_gTimTaskList2Arr[id][0];
		uc_gTimTaskList2Arr[id][us_lLen+2]=state;
		
		STMFLASH_Write(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*id)+4,(u16*)&state,1);
		
		if(state == 0){  //如果是由开启到关闭
			if(ul_gTimeTaskUnixTimArr[id] == RECENT_TASK_UTC){//如果当前任务就是最近需要执行的任务
				ul_gTimeTaskUnixTimArr[id] = 0;
				CalcMinTimstamp(); //
			}
			ul_gTimeTaskUnixTimArr[id] = 0;
		}else if(state == 1){ //从0变到1需要计算并计算出最近任务和设闹钟
			//取出strLen
			memcpy(&us_lLen,&uc_gTimTaskList2Arr[id][2],2);  //取出名称的string len
			//计算对应ID的时间戳
			if((uc_gTimTaskList2Arr[id][us_lLen + 4] & 0x80) == 0){
				CalcSpecificNormalTask(id);
			
			}else{
				CalcSpecificPeriodicTask(id,TRUE);
			}
			//找出最近需要执行的任务
			CalcMinTimstamp(); //
		}
		SUCCESS_REPORT();
	}
}
/**
  * @brief GetTimeTaskList  获取定时任务列表
  * @param  startPos 开始位置
	*					quantity 一次获取多少个任务列表
  * @retval None
  */
void GetTimeTaskList(u32 startPos,u32 quantity)
{
	u8 i,* volatile p;
	u16 us_lLen=0;
	xSemaphoreTake(MutexSemaphor,10);
	
	stp_gIntIntSend=(ST_IntIntS *)&stp_gWifiDataHeadToSend->next;
	stp_gIntIntSend->result=RESULT_TRUE;
	stp_gIntIntSend->taskID=st_gHostInfo.timeTaskCount; //这里的->taskID格式化的是任务总数
	stp_gIntIntSend->epilog=(u16)st_gHostInfo.timeTaskCount; //这里的->epilog格式化的是任务列表Array Len
	
	p=(u8 *)(&stp_gIntIntSend->epilog);
	p+=2;     //指针指向任务列表长度（数组长度）字段后加2 越过
	for(i=0;i<TIMINGTASK_LIST_LEN_MAX;i++){
		//把保存过后的头部添加进去
		if(strcmp((const char *)&uc_gTimTaskList2Arr[i][0],"")!=0){
			*(u32*)p=i+1;//存入任务ID(客户端不能识别0的ID所以这里ID加1)
			p+=4;			 //指针移动到任务ID后
			//提取出头部长度
			us_lLen=uc_gTimTaskList2Arr[i][1];//低放低，高放高
			us_lLen<<=8;
			us_lLen|=uc_gTimTaskList2Arr[i][0];
			memcpy(p,&uc_gTimTaskList2Arr[i][2],us_lLen+1);//头部长度没有计算状态 所以+1
			p+=us_lLen+1;//指针移动
		}
	}
	*(u16*)p=EPILOG; //把结束符添加进去
	
	st_gWifiReport.reportLen = findLen((u8*)st_gWifiReport.reportBuf);
	if( st_gWifiReport.reportLen > 10){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	xSemaphoreGive(MutexSemaphor);
}	
/**
  * @brief  Get_TimeTaskInfo 获取单个定时任务信息
  * @param 	taskID	所需要获取的任务ID
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
	p	+=	2;         //指针指向taskID后移动4个字节
	
	if( taskID > TIMINGTASK_LIST_LEN_MAX){
			ERROR_REPORT();
	}else{
		taskLen	=	STMFLASH_ReadHalfWord( TIME_LIST_FLASH_ADDR + ( TIMINGTASK_DATALEN_MAX * taskID ) ); //先取出总长度
		
		STMFLASH_Read( TIME_LIST_FLASH_ADDR + ( TIMINGTASK_DATALEN_MAX * taskID) + 6, p, ( taskLen + taskLen % 2 ) /2 ); //读出数据到st_gWifiReport.reportBuf
		
		st_gWifiReport.reportLen = findLen((u8*)st_gWifiReport.reportBuf);
		
		//读取状态到末尾
		st_gWifiReport.reportBuf[ st_gWifiReport.reportLen - 3 ] = (u8)(STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*taskID)+4));
		
		if( st_gWifiReport.reportLen > 10 ){
			Wifi_QueueIn( &st_gWifiReport ); //Wifi发送						
		}
	}
	xSemaphoreGive(MutexSemaphor);
}

/**
  * @brief  ExecuteTask 执行定时任务 只需要传入ID 函数根据ID读出命令组执行命令组 
  * @param 	taskID 传入需要执行的ID
  * @retval TRUE  执行成功
	*					FALSE	执行失败
  */

BOOL ExecuteTask(u8 taskID)
{
	u16 		us_headLen 		= 0;
	u16 		us_totalLen 	= 0;
	u16 		us_cmdArrLen 	= 0;
	u16 		us_cmdLen 		= 0;
	u8			*p						=NULL;
	uint64_t	ull_addr 		=	0;
	//根据ID把命令的长度读出来
	us_totalLen =	STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * taskID));
	us_headLen 	= STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * taskID) + 2);
	if((us_totalLen > TIMINGTASK_DATALEN_MAX) || (us_headLen > TIMINGTASK_HEAD_LEN_MAX)){
		return FALSE;
	}
	//把整个命令组都读出来 
	STMFLASH_Read(TIME_LIST_FLASH_ADDR+(TIMINGTASK_DATALEN_MAX*taskID)+6,(u16 *)uc_gTimCmdArr,(us_totalLen+us_totalLen%2)/2);
	//取出执行命令数组的长度（）
	//	us_cmdArrLen = uc_gTimCmdArr[us_headLen]; //取低位也可以因为命令数组不可能大于255  取一个16位的更安全
	memcpy((void *)&us_cmdArrLen,&uc_gTimCmdArr[us_headLen],2);
	//指针指向 "执行设备ID" +2 越过“命令数组长度”
	p = &uc_gTimCmdArr[us_headLen+2];
	
	//由于只为任务开辟了一个 TIMINGTASK_DATALEN_MAX 个字节的空间  所有命令不得大于 TIMINGTASK_CMD_MAX
	if(us_cmdArrLen <= TIMINGTASK_CMD_MAX){
		while(us_cmdArrLen){
			
			memcpy(&ull_addr,p,8);
			//+8越过执行设备ID
			p += 8;
			//复制出16位的 命令码长度
			memcpy((void *)&us_cmdLen,p,2);
			//越过 命令码长度  指向命令内容
			p	+= 2;
			
			//检查设备地址是不是本地址 (本地址就直接执行)
			if(st_gHostInfo.deviceID == ull_addr){
				//执行命令
				CONTROL_FUN(p,us_cmdLen);
			
			}else{ //非本地址就发送到相应地址
				
				CONTROL_SLAVE_FUN((u32)ull_addr, us_cmdLen, p);
			}
			p	+= us_cmdLen;		//越过“命令码内容”
			p += 2;						//越过“执行延时”
			us_cmdArrLen--;
			
		}
		return TRUE;
	}else{
		//_Error_Handler(__FILE__, __LINE__);
		return FALSE;
	}
}
/**
  * @brief  DataInit	初始化数据 主机状态等数据，定时任务数据，分机和开关 地址数据
  * @param 
  * @retval None
  */
void DataInit(void)
{
	u16 us_taskHeadLen;
	u16 us_tasktotalLen;
	u8 	i;
	//获取st_gHostInfo中需要获取的数据
//	st_gHostInfo.deviceID=
	//获取锁定状态 主机是否已锁定（锁定后不可被00地址广播）
	st_gHostInfo.lockInfo 		= (u8) STMFLASH_ReadHalfWord(LOCK_FLAG_FLASH_ADDR);
	st_gHostInfo.version			=	VER;
	st_gHostInfo.slaveCount		=	0;
	st_gHostInfo.switchCount	=	0;
	st_gHostInfo.timeTaskCount=	0;
	if((st_gHostInfo.lockInfo == 0xFF) || st_gHostInfo.lockInfo == 0)  st_gHostInfo.lockInfo = 0x01;
	//取出主机设备地址 64位
//	st_gHostInfo.deviceID			= STMFLASH_ReadHalfWord(DEVIDH_FLASH_ADDR + 2); //取出本身芯片的UID高32位
//	st_gHostInfo.deviceID		<<=	16; //左移16位
//	st_gHostInfo.deviceID		 |=	STMFLASH_ReadHalfWord(DEVIDH_FLASH_ADDR);
//	st_gHostInfo.deviceID		<<=	16; //左移16位
//	st_gHostInfo.deviceID		 |=	STMFLASH_ReadHalfWord(ROLLING_CODE_ADDR + 2); //取出滚动码的4个字节
//	st_gHostInfo.deviceID		<<=	16; //左移16位
//	st_gHostInfo.deviceID		 |=	STMFLASH_ReadHalfWord(ROLLING_CODE_ADDR);
		st_gHostInfo.deviceID = ((uint64_t)((*(__IO u32*)DEVIDL_FLASH_ADDR)|(*(__IO u32*)DEVIDH_FLASH_ADDR)) << 32) |((*(__IO u32*)DEVIDM_FLASH_ADDR)|(*(__IO u32*)DEVIDH_FLASH_ADDR)); 
	//获取已添加过的分机并计数
	for(i = 0;i < SLAVE_LIST_MAX;i++){
		STMFLASH_Read(SLAVE_LIST_FLASH_ADDR + (SLAVE_LIST_FLASH_INVL * i),	(u16 *)( & st_gHostInfo.stSlave[i].slaveID),	2);
		if(st_gHostInfo.stSlave[i].slaveID == 0xFFFFFFFF){
			st_gHostInfo.stSlave[i].slaveID	= 0;
		}else if(st_gHostInfo.stSlave[i].slaveID == 0){
			;//Do nothing
		}else{
			st_gHostInfo.stSlave[i].slaveState = 1;
			st_gHostInfo.slaveCount++;  //既不等于0也不全是FF说明这个是有数据的。+1
		}
	}
	//获取已添加过的开关并计数
	for(i = 0;i < SWITCH_LIST_MAX;i++){
		STMFLASH_Read(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i),	(u16 *)( & st_gHostInfo.stSwitch[i].switchID),	2);
		if(st_gHostInfo.stSwitch[i].switchID == 0xFFFFFFFF){
			st_gHostInfo.stSwitch[i].switchID	= 0;
		}else if(st_gHostInfo.stSwitch[i].switchID == 0){
			;//Do nothing
		}else{
			//读出开关的位数并且计数加1
			st_gHostInfo.stSwitch[i].channle = (u8)STMFLASH_ReadHalfWord(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i) + 4);
			st_gHostInfo.switchCount++;  //既不等于0也不全是FF说明这个是有数据的。+1
		}
	}
	//获取定时任务数据
	for(i = 0; i< TIMINGTASK_LIST_LEN_MAX; i++){
		//取出头部长度和总长度
		us_tasktotalLen	=	STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * i));
		us_taskHeadLen 	= STMFLASH_ReadHalfWord(TIME_LIST_FLASH_ADDR + (TIMINGTASK_DATALEN_MAX * i) + 2);
		
		if((us_tasktotalLen == 0xFFFF) || (us_taskHeadLen == 0xFFFF)){
			us_tasktotalLen = 0;
			us_taskHeadLen	=	0;
		}
		//如果任务列表在FLASH中有数据就读到任务列表数组中
		if(us_taskHeadLen != 0){
			memcpy(&uc_gTimTaskList2Arr[i][0], &us_taskHeadLen, 2);//把2个字节的头部长度放到任务列表数组中去
					//(us_taskHeadLen + us_taskHeadLen%2)/2  取半字
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
  * @brief  EraseDevice 擦除WIFI模块连接信息和设备Flash
  * @param 
  * @retval None
  */
void EraseDevice(void)
{
	u8 	i;
	//清除锁定状态
	st_gHostInfo.lockInfo 		= 1; //1代表在线 0表示不在线 2表示锁定
	STMFLASH_Erase_Word(LOCK_FLAG_FLASH_ADDR);
	
	st_gHostInfo.version			=	VER;
	st_gHostInfo.slaveCount		=	0;
	st_gHostInfo.switchCount	=	0;
	st_gHostInfo.timeTaskCount=	0;
	
	for(i = 0;i < SLAVE_LIST_MAX;i++){
		//清除开关数据 如果不为空
		if(st_gHostInfo.stSwitch[i].switchID != 0){
			st_gHostInfo.stSwitch[i].switchID = 0;
			st_gHostInfo.stSwitch[i].channle	=	0;
			STMFLASH_Erase_Word(SWITCH_LIST_FLASH_ADDR + (SWITCH_LIST_FLASH_INVL * i));
		}
		//清除分机数据 如果不为空
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
