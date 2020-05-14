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
u8										uc_gRFSendBufferArr[RF433BUFF_MAX];  //RF���ͻ���
ST_433Packet * const	stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr;
volatile	u8					n_gIDCount; //RF433ÿ�η��͵�ID��һ����Ҫ++


extern ST_HostInfo		st_gHostInfo; 	//������Ϣ �����ҵ�����������豸��Ϣ
/**ѧϰ���**/
volatile u8 	 				uc_vgTim2CountMode; //TIM2��uc_vgTim2CountMode�ļ���������increase����decrease
volatile u8 	 				uc_vgStudyMode;
volatile u8						uc_vgStudyTim1Out;

extern ST_RFIRStudy 	st_gRFStudyData; //ѧϰ�ṹ��


//extern u8 volatile 	n_gWifiSend;		//�Ƿ���ҪWifi����


/*��Ϣ���о������RF������Ϣ���ر����ڶ�ʱ��������Ҫ���Ͷ�� 5�����У�ÿ��������64���ֽڣ�*/
extern QueueHandle_t RFSendQueueHandle;  //����Ϣ���д���USART���ݵ�����

/**
  * @brief  SetDefaultSYNC ����RF ��sync wordΪ������ַ
  * @param 
  * @retval None
  */
void SetDefaultSYNC(void)
{
	Cmt2300_GoStby();
	Cmt2300_SetSynWord((u32)(st_gHostInfo.deviceID)); //��synWord��Ϊ������ַ
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
//  stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr; //ǿָ���ṹ��
	stp_g433Packet->SoucreAddr=st_gHostInfo.deviceID;
	stp_g433Packet->TargetADDR=SYNC_WORD_ADD;
	stp_g433Packet->Type=0;
	stp_g433Packet->Cmd=RF_ADD_SLAVE;
	stp_g433Packet->Id=n_gIDCount;
	stp_g433Packet->Len=0x00;
	
	Cmt2300_GoStby();
	Cmt2300_SetResendTime(2);
//	Cmt2300_SetSynWord(SYNC_WORD_ADD); //ֱ������CMT2300��SYNword
//						 tmp_addr=Cmt2300_ReadSynWord();
	Cmt2300_GoSleep();
//	portEXIT_CRITICAL();
	//��CMT������������Ϣ֪ͨ ���Կ�ʼ������  //�������ſ����ж����ж�����CMT�����Ϳɽ���
	
	if(RFSendQueueHandle != NULL){ //���433���� ��Ϣ���б��ɹ�������
		if(xQueueSend(RFSendQueueHandle,(void *)uc_gRFSendBufferArr,10)!= pdPASS){
			;//���������з���ʧ����
		}
	}
}
/**
  * @brief	RFGenericSend		���ߵ�ͨ�÷��亯��
  * @param 	targetAddr��Ŀ���ַ
	* @param	cmd:				��������
	*	@param	len:				���ݳ��ȣ�ֻ�������ݲ�����ǰ��ĸ�ʽ��
	*	@param	content:		�������ݵ�ָ��
  * @retval None
  */
void RFGenericSend(u32 targetAddr,u8 cmd,u8 len,const u8 * content)
{
//	ST_SwitchState *	stp_lSwitchState;
	n_gIDCount++;
	if(n_gIDCount==0) n_gIDCount=1;
//	portENTER_CRITICAL();
	memset(uc_gRFSendBufferArr,0,RF433BUFF_MAX);
//  stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr; //ǿָ���ṹ��
	stp_g433Packet->SoucreAddr=(u32)st_gHostInfo.deviceID;
	stp_g433Packet->TargetADDR=targetAddr;
	stp_g433Packet->Type=0;
	stp_g433Packet->Cmd=cmd;
	stp_g433Packet->Id=n_gIDCount;
	stp_g433Packet->Len=len;
	if(len>0){
//		stp_lSwitchState=(ST_SwitchState*)&stp_g433Packet->Data; //������ȴ���0��ô��������Ӧ��������
//		stp_lSwitchState->switchChannel	=data0;
//		stp_lSwitchState->switchState		=data1;
		memcpy(&stp_g433Packet->Data,content,len);
	}
	
//	portEXIT_CRITICAL();

	//��CMT������������Ϣ֪ͨ ���Կ�ʼ������  //�������ſ����ж����ж�����CMT�����Ϳɽ���
	
	if(RFSendQueueHandle != NULL){ //���433���� ��Ϣ���б��ɹ�������
		if(xQueueSend(RFSendQueueHandle,(void *)uc_gRFSendBufferArr,10)!= pdPASS){
			_Error_Handler(__FILE__,__LINE__);//���������з���ʧ����
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
		
			uc_vgTim2CountMode	=	INCREASE;  //������ģʽ �����ӵķ�ʽ
			uc_vgStudyMode			=	STUDY_MODE_IR; //ѧϰģʽ��IR
			uc_vgStudyTim1Out		=	STUDY_TIME_MAX;			//ѧϰ��ʱ���������� ���� == 1��ʱ���ʧ��
		
			memset(&st_gRFStudyData,0,sizeof(st_gRFStudyData));
			
			st_gRFStudyData.Type =	studyType;
			
			__HAL_TIM_ENABLE(&htim2); //�򿪶�ʱ��2���жϣ�����ѧϰ
//		  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //�򿪺���ѧϰ��IO���жϿ�ʼѧϰ
			
//			portEXIT_CRITICAL();
			break;
		
		case STUDY_RF433 :
//			portENTER_CRITICAL();
		
			RF_Init(RF_OOK_RX); //��CMT2300Aģ���ʼ����OOKģʽ
			uc_vgTim2CountMode=INCREASE; //���ѧϰ״̬����ѧϰ״̬����OOK���ж��в���ѧϰ
			uc_vgStudyMode=STUDY_MODE_RF;    //ѧϰģʽ��RF
		
			memset(&st_gRFStudyData,0,sizeof(st_gRFStudyData));
		
			st_gRFStudyData.Type=studyType; //��Ҫ���ظ��ͻ���ѧϰ������
			__HAL_TIM_ENABLE(&htim2); //������ʱ��
		
//			portEXIT_CRITICAL();
			break;
		
		case STUDY_RF315 :
			break;
		default :
			break;
	}
	st_gRFStudyData.LibCHK=0x5AA5; //���һ����ͨ��ѧϰ����������
}
/**
  * @brief  StopStudy ֹͣѧϰ
  * @param 	None
  * @retval None
  */
void StopStudy(void)
{
	__HAL_TIM_DISABLE(&htim2); //�رն�ʱ��2(50us ���������)���ж�ֹͣѧϰ
//	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);//�رպ����Ӧ�ж�
	uc_vgTim2CountMode = 0;
	
	if(uc_vgStudyMode==STUDY_MODE_RF){ //���
		RF_Init_FSK();
	}
//	WifiIntReport(&stp_gWifiDataHeadToSend->next,RESULT_TRUE);
}
/******************************�������亯������ظ�������***********************************/
void SwitchControlBack()
{
	
}
/**
  * @brief ȷ�ϻظ� 0x23;
  * @param None
  * @retval None
  */
void Confirm(u32 targetAddr,u8 rf_cmd)
{
	n_gIDCount++;
	if(n_gIDCount==0) n_gIDCount=1;
	portENTER_CRITICAL();
	memset(uc_gRFSendBufferArr,0,RF433BUFF_MAX);
//  stp_g433Packet=(ST_433Packet *)uc_gRFSendBufferArr; //ǿָ���ṹ��
	stp_g433Packet->SoucreAddr=st_gHostInfo.deviceID;
	stp_g433Packet->TargetADDR=targetAddr;
	stp_g433Packet->Type=0;
	stp_g433Packet->Cmd=rf_cmd; //0x23ȷ������ 
	stp_g433Packet->Id=n_gIDCount;
	stp_g433Packet->Len=0x00;
	portEXIT_CRITICAL();
	Cmt2300_GoStby();
	Cmt2300_SetResendTime(40);
//	Cmt2300_SetSynWord(SYNC_WORD_ADD); //ֱ������CMT2300��SYNword
//						 tmp_addr=Cmt2300_ReadSynWord();
	Cmt2300_GoSleep();
	//��CMT������������Ϣ֪ͨ ���Կ�ʼ������  //�������ſ����ж����ж�����CMT�����Ϳɽ���
	
//	if(RFSendQueueHandle != NULL){ //���433���� ��Ϣ���б��ɹ�������
//		if(xQueueSend(RFSendQueueHandle,(void *)uc_gRFSendBufferArr,10)!= pdPASS){
//			;//���������з���ʧ����
//		}
//	}
	RF_Send(uc_gRFSendBufferArr,stp_g433Packet->Len+12);
}
/******************************����CMT2300A�ظ�������*************************************/


/*************************��������Ϊִ�к�����ִ�к����OOK����****************************************/
/**
  * @brief  Send433Byte  ����433��ÿ���ֽڴ���
  * @param  uc_argByte:  ��Ҫ���͵��ֽ�
	*	@param	stp_argRFStudy: ������Ƶ�Զ���ѧϰ��ʽ �ṹ��ָ��
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
  * @brief  RFSendStudy ����ѧϰ����RF433����
  * @param  stp_argRFStudy: ������Ƶ�Զ���ѧϰ��ʽ �ṹ��ָ��
  * @retval None
  */
static void RFSendStudy(ST_RFIRStudy *stp_argRFStudy)
{
	u8 i,n_lSenCount;
//  stp_gSendRFStudy=(ST_RFIRStudy *)next;
	RF_Init(RF_OOK_TX);
	uc_vgTim2CountMode=DECREASE;  //����ѧϰ��ʱ����Ҫ�������ݼ�
	__HAL_TIM_ENABLE(&htim2); 
	for(n_lSenCount=0;n_lSenCount<3;n_lSenCount++){
		/*ͷ���*/
		RF433OUT(1);
		us_vgPulseCnt=stp_argRFStudy->HeadHCnt;
		while(us_vgPulseCnt>0);
		/*ͷ���*/
		RF433OUT(0);
		us_vgPulseCnt=stp_argRFStudy->HeadLCnt;
		while(us_vgPulseCnt>0);
		/*����*/

		for(i=0; i< stp_argRFStudy->Len; i++){
			Send433Byte(stp_argRFStudy,stp_argRFStudy->Data[i]);
		}
		delay_ms(10);
	}
	__HAL_TIM_DISABLE(&htim2);  //�ض�ʱ��2
	uc_vgTim2CountMode=0;						//��Ƿ���OOK����
	RF_Init_FSK();
}




/**
  * @brief GenericSendStudy  �ж���ʲô���ͣ���ѡ������IR ���� 433
  * @param next���ݿ�ʼ�ĵط�������Ӧ���ӿ����������string len ������ݿ�ʼ��
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
  * @brief  SendControlCmd ���Ϳ�������
  * @param 	next	���ݿ�ʼ�ĵط�������Ӧ���ӿ����������string len ������ݿ�ʼ��
	* @param	len : ���ݵĳ���
  * @retval None
  */
void SendControlCmd(const u8 *next,u8 len)
{
	ST_RFIRStudy	*		stp_RFIRStudy;
	stp_RFIRStudy = (ST_RFIRStudy *)next;
	
	if(stp_RFIRStudy->LibCHK == CUSTOM_LIB){ //������Զ�������Զ���Ľ���
		GenericSendStudy(stp_RFIRStudy);
	}else{													//���������ķ�ʽ����
		DisCode_Ir(next,len);
		SendIRFormlib();
		WifiIntCharReport(1);
	}
}
/***********************************************************************************/

