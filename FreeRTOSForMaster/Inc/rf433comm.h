#ifndef __RF433COMM_H
#define __RF433COMM_H
#include "sys.h"

/*RF的最大发送次数*/
#define RF433SEND_MAX         3 
#define RF433BUFF_MAX					64
/*添加分机的sync word*/
#define SYNC_WORD_ADD          0xFFFFFFFF

#define RF_OOK_RX		1U //在RF_Init初始化到OOK的时候是默认发送还是默认接收
#define RF_OOK_TX		2U



#define CMT_SENDBIT			(1<<1)
#define CMT_RECEIVEBIT	(1<<2)
#define SENDANDRECE	(SENDBIT | RECEIVEBIT)
#define RESET_NOTIFI_VALUE	0xffffffffUL  //清除任务通知的值

#define RF_SWITCH_CONTROL		0x11  //控制开关和控制开关后+0x80返回
#define RF_SWITCH_REPORT		0x12	//开关主动+0x80上报
#define RF_ADD_SLAVE				0x21
#define RF_ADD_SWITCH				0x31
#define RF_CONTROL_DEVICE		0x25
#define RF_DEL_SLAVE 				0x26
#define RF_DEL_SWITCH				0x27
//通用确认回复
#define RF_CONFIRM					0x23
//开关主动上报的确认回复
#define RF_CONFIRM_SWITCH		0x22

/**
  *433发送和接收结构体
  */
typedef struct 
{
	u32 SoucreAddr; //源地址
	u32 TargetADDR;   //目标地址
  u8 Type;        //设备类型
	u8 Cmd;         //命令类型  0x00-主机  0x01-中继 0x02-开关
	u8 Id;
	u8 Len;         //数据长度
	u8 Data;   //数据
}ST_433Packet;

typedef struct
{
	u8 switchState;
	u8 switchChannel;
}ST_SwitchState;

/**
  *红外射频学习数据体
  */
typedef struct //红外学习体数据结构-28
{
					 u16 LibCHK;  //码库检查，检查从客户端发下来的控制数据是码库还是我们学习的码 固定为0x5AA5;
	volatile u16 HeadHCnt;//头码高电平时延，n*50us
	volatile u16 HeadLCnt;//头码低电平时延，n*50us
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
  *学习进度（状态）的枚举列表
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
  *脉冲宽度的比较结果枚举
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

extern u8							uc_gRFSendBufferArr[RF433BUFF_MAX];  //RF发送缓存
extern volatile u8 	 uc_vgTim2CountMode; 
//ST_433Packet * const	st_g433Packet;
#endif
