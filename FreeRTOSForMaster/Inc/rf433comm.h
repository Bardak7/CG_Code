#ifndef __RF433COMM_H
#define __RF433COMM_H
#include "sys.h"

/*RF������ʹ���*/
#define RF433SEND_MAX         3 
#define RF433BUFF_MAX					64
/*��ӷֻ���sync word*/
#define SYNC_WORD_ADD          0xFFFFFFFF

#define RF_OOK_RX		1U //��RF_Init��ʼ����OOK��ʱ����Ĭ�Ϸ��ͻ���Ĭ�Ͻ���
#define RF_OOK_TX		2U



#define CMT_SENDBIT			(1<<1)
#define CMT_RECEIVEBIT	(1<<2)
#define SENDANDRECE	(SENDBIT | RECEIVEBIT)
#define RESET_NOTIFI_VALUE	0xffffffffUL  //�������֪ͨ��ֵ

#define RF_SWITCH_CONTROL		0x11  //���ƿ��غͿ��ƿ��غ�+0x80����
#define RF_SWITCH_REPORT		0x12	//��������+0x80�ϱ�
#define RF_ADD_SLAVE				0x21
#define RF_ADD_SWITCH				0x31
#define RF_CONTROL_DEVICE		0x25
#define RF_DEL_SLAVE 				0x26
#define RF_DEL_SWITCH				0x27
//ͨ��ȷ�ϻظ�
#define RF_CONFIRM					0x23
//���������ϱ���ȷ�ϻظ�
#define RF_CONFIRM_SWITCH		0x22

/**
  *433���ͺͽ��սṹ��
  */
typedef struct 
{
	u32 SoucreAddr; //Դ��ַ
	u32 TargetADDR;   //Ŀ���ַ
  u8 Type;        //�豸����
	u8 Cmd;         //��������  0x00-����  0x01-�м� 0x02-����
	u8 Id;
	u8 Len;         //���ݳ���
	u8 Data;   //����
}ST_433Packet;

typedef struct
{
	u8 switchState;
	u8 switchChannel;
}ST_SwitchState;

/**
  *������Ƶѧϰ������
  */
typedef struct //����ѧϰ�����ݽṹ-28
{
					 u16 LibCHK;  //����飬���ӿͻ��˷������Ŀ�����������⻹������ѧϰ���� �̶�Ϊ0x5AA5;
	volatile u16 HeadHCnt;//ͷ��ߵ�ƽʱ�ӣ�n*50us
	volatile u16 HeadLCnt;//ͷ��͵�ƽʱ�ӣ�n*50us
	volatile u16 EndHCnt;
	volatile u16 EndLCnt;
	volatile u16 Data1HCnt;  
	volatile u16 Data1LCnt;
	volatile u16 Data0HCnt;
	volatile u16 Data0LCnt;
	volatile u8 Type;  //ir-1,433-2,315-3
	volatile u8 Len;
	volatile u8 Data[50];
}ST_RFIRStudy;
/**
  *ѧϰ���ȣ�״̬����ö���б�
  */
typedef enum
{
	waitHeadH = 0,
	waitHeadL,
	waitDataH,
	waitDataL,
	genericWait,
	studyOK,
	badCode
}EM_StudyState;
/**
  *�����ȵıȽϽ��ö��
  */
typedef enum{
	nonuse,
	equal,
	unequal
}EM_PulseWidth;

void SetDefaultSYNC(void);
void Confirm(u32 targetAddr,u8 rf_cmd);
void RFGenericSend(u32 targetAddr,u8 cmd,u8 len,const u8 * content);
void GenericStudy(u8 studyType);
void StopStudy(void);

//void RFSendStudy(ST_RFIRStudy *stp_argRFStudy);
//void GenericSendStudy(const u8 *next);
void SendControlCmd(const u8 *next,u8 len);

extern u8							uc_gRFSendBufferArr[RF433BUFF_MAX];  //RF���ͻ���
extern volatile u8 	 uc_vgTim2CountMode; 
//ST_433Packet * const	st_g433Packet;
#endif
