#include "rf433comm.h"
#include "protocol.h"
#include "cmt2300.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "string.h"
#include "radio.h"
#include "gpio.h"
#include "tim.h"
#include "ir.h"
#include "main.h"
u8										uc_gRFSendBufferArr[RF433BUFF_MAX];  //RF发送缓存
ST_433Packet * const	stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr;
volatile	u8					n_gIDCount; //RF433每次发送的ID不一样需要++


extern ST_HostInfo		st_gHostInfo; 	//主机信息 包含挂到主机下面的设备信息
/**学习相关**/
volatile u8 	 				uc_vgTim2CountMode; //TIM2中uc_vgTim2CountMode的计数方向是increase还是decrease
volatile u8 	 				uc_vgStudyMode;
volatile u8						uc_vgStudyTim1Out;

extern ST_RFIRStudy 	st_gRFStudyData; //学习结构体


//extern u8 volatile 	n_gWifiSend;		//是否需要Wifi发送


/*消息队列句柄用于RF发送消息（特别是在定时任务中需要发送多个 5个队列，每个队列有64个字节）*/
extern QueueHandle_t RFSendQueueHandle;  //用消息队列传送USART数据到任务

/**
  * @brief  SetDefaultSYNC 设置RF 的sync word为主机地址
  * @param 
  * @retval None
  */
void SetDefaultSYNC(void)
{
	Cmt2300_GoStby();
	Cmt2300_SetSynWord((u32)(st_gHostInfo.deviceID)); //把synWord改为主机地址
	Cmt2300_GoSleep();
	RF_Receive();
}
/**
  * @brief  AddSlave
  * @param  None
  * @retval None
  */
void AddSlave()
{
	n_gIDCount++;
	if(n_gIDCount==0) n_gIDCount=1;
//	portENTER_CRITICAL();
	memset(uc_gRFSendBufferArr,0,RF433BUFF_MAX);
//  stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr; //强指到结构体
	stp_g433Packet->SoucreAddr=st_gHostInfo.deviceID;
	stp_g433Packet->TargetADDR=SYNC_WORD_ADD;
	stp_g433Packet->Type=0;
	stp_g433Packet->Cmd=RF_ADD_SLAVE;
	stp_g433Packet->Id=n_gIDCount;
	stp_g433Packet->Len=0x00;
	
	Cmt2300_GoStby();
	Cmt2300_SetResendTime(2);
//	Cmt2300_SetSynWord(SYNC_WORD_ADD); //直接设置CMT2300的SYNword
//						 tmp_addr=Cmt2300_ReadSynWord();
	Cmt2300_GoSleep();
//	portEXIT_CRITICAL();
	//给CMT处理任务发送消息通知 可以开始发送了  //接收引脚可做中断在中断中向CMT任务发送可接收
	
	if(RFSendQueueHandle != NULL){ //如果433发送 消息队列被成功建立了
		if(xQueueSend(RFSendQueueHandle,(void *)uc_gRFSendBufferArr,10)!= pdPASS){
			;//如果向队列中发送失败了
		}
	}
}
/**
  * @brief	RFGenericSend		无线的通用发射函数
  * @param 	targetAddr：目标地址
	* @param	cmd:				命令类型
	*	@param	len:				数据长度（只包含数据不包含前面的格式）
	*	@param	content:		数据内容的指针
  * @retval None
  */
void RFGenericSend(u32 targetAddr,u8 cmd,u8 len,const u8 * content)
{
//	ST_SwitchState *	stp_lSwitchState;
	n_gIDCount++;
	if(n_gIDCount==0) n_gIDCount=1;
//	portENTER_CRITICAL();
	memset(uc_gRFSendBufferArr,0,RF433BUFF_MAX);
//  stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr; //强指到结构体
	stp_g433Packet->SoucreAddr=(u32)st_gHostInfo.deviceID;
	stp_g433Packet->TargetADDR=targetAddr;
	stp_g433Packet->Type=0;
	stp_g433Packet->Cmd=cmd;
	stp_g433Packet->Id=n_gIDCount;
	stp_g433Packet->Len=len;
	if(len>0){
//		stp_lSwitchState=(ST_SwitchState*)&stp_g433Packet->Data; //如果长度大于0那么数据体是应该有数据
//		stp_lSwitchState->switchChannel	=data0;
//		stp_lSwitchState->switchState		=data1;
		memcpy(&stp_g433Packet->Data,content,len);
	}
	
//	portEXIT_CRITICAL();

	//给CMT处理任务发送消息通知 可以开始发送了  //接收引脚可做中断在中断中向CMT任务发送可接收
	
	if(RFSendQueueHandle != NULL){ //如果433发送 消息队列被成功建立了
		if(xQueueSend(RFSendQueueHandle,(void *)uc_gRFSendBufferArr,10)!= pdPASS){
			_Error_Handler(__FILE__,__LINE__);//如果向队列中发送失败了
		}
	}else{
			_Error_Handler(__FILE__,__LINE__);
	}
}

void GenericStudy(u8 studyType)
{
	
	switch(studyType){
		case STUDY_IR :
//			portENTER_CRITICAL();
		
			uc_vgTim2CountMode	=	INCREASE;  //计数器模式 是增加的方式
			uc_vgStudyMode			=	STUDY_MODE_IR; //学习模式是IR
			uc_vgStudyTim1Out		=	STUDY_TIME_MAX;			//学习超时计数器重置 当它 == 1的时候便失败
		
			memset(&st_gRFStudyData,0,sizeof(st_gRFStudyData));
			
			st_gRFStudyData.Type =	studyType;
			
			__HAL_TIM_ENABLE(&htim2); //打开定时器2的中断，用于学习
//		  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //打开红外学习的IO口中断开始学习
			
//			portEXIT_CRITICAL();
			break;
		
		case STUDY_RF433 :
//			portENTER_CRITICAL();
		
			RF_Init(RF_OOK_RX); //把CMT2300A模块初始化成OOK模式
			uc_vgTim2CountMode=INCREASE; //标记学习状态，在学习状态下在OOK的中断中才能学习
			uc_vgStudyMode=STUDY_MODE_RF;    //学习模式是RF
		
			memset(&st_gRFStudyData,0,sizeof(st_gRFStudyData));
		
			st_gRFStudyData.Type=studyType; //需要返回给客户端学习的类型
			__HAL_TIM_ENABLE(&htim2); //开启定时器
		
//			portEXIT_CRITICAL();
			break;
		
		case STUDY_RF315 :
			break;
		default :
			break;
	}
	st_gRFStudyData.LibCHK=0x5AA5; //标记一下是通过学习得来的数据
}
/**
  * @brief  StopStudy 停止学习
  * @param 	None
  * @retval None
  */
void StopStudy(void)
{
	__HAL_TIM_DISABLE(&htim2); //关闭定时器2(50us 脉宽计数器)的中断停止学习
//	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);//关闭红外对应中断
	uc_vgTim2CountMode = 0;
	
	if(uc_vgStudyMode==STUDY_MODE_RF){ //如果
		RF_Init_FSK();
	}
//	WifiIntReport(&stp_gWifiDataHeadToSend->next,RESULT_TRUE);
}
/******************************以下区间函数处理回复的数据***********************************/
void SwitchControlBack()
{
	
}
/**
  * @brief 确认回复 0x23;
  * @param None
  * @retval None
  */
void Confirm(u32 targetAddr,u8 rf_cmd)
{
	n_gIDCount++;
	if(n_gIDCount==0) n_gIDCount=1;
	portENTER_CRITICAL();
	memset(uc_gRFSendBufferArr,0,RF433BUFF_MAX);
//  stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr; //强指到结构体
	stp_g433Packet->SoucreAddr=st_gHostInfo.deviceID;
	stp_g433Packet->TargetADDR=targetAddr;
	stp_g433Packet->Type=0;
	stp_g433Packet->Cmd=rf_cmd; //0x23确认命令 
	stp_g433Packet->Id=n_gIDCount;
	stp_g433Packet->Len=0x00;
	portEXIT_CRITICAL();
	Cmt2300_GoStby();
	Cmt2300_SetResendTime(40);
//	Cmt2300_SetSynWord(SYNC_WORD_ADD); //直接设置CMT2300的SYNword
//						 tmp_addr=Cmt2300_ReadSynWord();
	Cmt2300_GoSleep();
	//给CMT处理任务发送消息通知 可以开始发送了  //接收引脚可做中断在中断中向CMT任务发送可接收
	
//	if(RFSendQueueHandle != NULL){ //如果433发送 消息队列被成功建立了
//		if(xQueueSend(RFSendQueueHandle,(void *)uc_gRFSendBufferArr,10)!= pdPASS){
//			;//如果向队列中发送失败了
//		}
//	}
	RF_Send(uc_gRFSendBufferArr,stp_g433Packet->Len+12);
}
/******************************处理CMT2300A回复的数据*************************************/


/*************************以下区间为执行函数，执行红外或OOK发送****************************************/
/**
  * @brief  Send433Byte  发送433的每个字节处理
  * @param  uc_argByte:  需要发送的字节
	*	@param	stp_argRFStudy: 红外射频自定义学习格式 结构体指针
  * @retval None
  */
static void Send433Byte(ST_RFIRStudy *stp_argRFStudy,u8 uc_argByte)
{
	u8 i;
	
	for(i=0;i<8;i++)
	{
		if(uc_argByte & 0x80)
		{
			RF433OUT(1);
			us_vgPulseCnt=stp_argRFStudy->Data1HCnt;
			while(us_vgPulseCnt>0);
			
			RF433OUT(0);
			us_vgPulseCnt=stp_argRFStudy->Data1LCnt;
			while(us_vgPulseCnt>0);
		}
		else
		{
			RF433OUT(1);
			us_vgPulseCnt=stp_argRFStudy->Data0HCnt;
			while(us_vgPulseCnt>0);
			
			RF433OUT(0);
			us_vgPulseCnt=stp_argRFStudy->Data0LCnt;
			while(us_vgPulseCnt>0);
		}
		uc_argByte = uc_argByte << 1;
	}
}

/**
  * @brief  RFSendStudy 发送学习到的RF433数据
  * @param  stp_argRFStudy: 红外射频自定义学习格式 结构体指针
  * @retval None
  */
static void RFSendStudy(ST_RFIRStudy *stp_argRFStudy)
{
	u8 i,n_lSenCount;
//  stp_gSendRFStudy=(ST_RFIRStudy *)next;
	RF_Init(RF_OOK_TX);
	uc_vgTim2CountMode=DECREASE;  //发送学习的时候是要计数器递减
	__HAL_TIM_ENABLE(&htim2); 
	for(n_lSenCount=0;n_lSenCount<3;n_lSenCount++){
		/*头码高*/
		RF433OUT(1);
		us_vgPulseCnt=stp_argRFStudy->HeadHCnt;
		while(us_vgPulseCnt>0);
		/*头码低*/
		RF433OUT(0);
		us_vgPulseCnt=stp_argRFStudy->HeadLCnt;
		while(us_vgPulseCnt>0);
		/*数据*/

		for(i=0; i< stp_argRFStudy->Len; i++){
			Send433Byte(stp_argRFStudy,stp_argRFStudy->Data[i]);
		}
		delay_ms(10);
	}
	__HAL_TIM_DISABLE(&htim2);  //关定时器2
	uc_vgTim2CountMode=0;						//标记发送OOK结束
	RF_Init_FSK();
}




/**
  * @brief GenericSendStudy  判断是什么类型，再选择是用IR 或者 433
  * @param next数据开始的地方（数据应当从控制命令码的string len 后的内容开始）
  * @retval None
  */
static void GenericSendStudy(ST_RFIRStudy *stp_argRFStudy)
{
	
	switch(stp_argRFStudy->Type){
		case STUDY_IR :
			IRSendStudy(stp_argRFStudy);
		 break;
		case STUDY_RF433 :
			RFSendStudy(stp_argRFStudy);
			break;
		case STUDY_RF315 :
			break;
		default :
			
			break;
		 
	}
	WifiIntCharReport(1);
}
/**
  * @brief  SendControlCmd 发送控制命令
  * @param 	next	数据开始的地方（数据应当从控制命令码的string len 后的内容开始）
	* @param	len : 数据的长度
  * @retval None
  */
void SendControlCmd(const u8 *next,u8 len)
{
	ST_RFIRStudy	*		stp_RFIRStudy;
	stp_RFIRStudy = (ST_RFIRStudy *)next;
	
	if(stp_RFIRStudy->LibCHK == CUSTOM_LIB){ //如果是自定义的用自定义的解码
		GenericSendStudy(stp_RFIRStudy);
	}else{													//否则用码库的方式解码
		DisCode_Ir(next,len);
		SendIRFormlib();
		WifiIntCharReport(1);
	}
}
/***********************************************************************************/

