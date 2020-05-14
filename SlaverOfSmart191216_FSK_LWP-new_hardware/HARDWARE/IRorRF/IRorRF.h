#ifndef __IRorRF_H
#define __IRorRF_H	

#include "typedefs.h"
#include "sys.h"

//typedef struct 
//{
//	u32 SoucreAddr; //源地址
//	u32 DistAddr;   //目标地址
//  u8 Type;        //设备类型
//	u8 Cmd;         //命令类型  0x00-主机  0x01-中继 0x02-开关
//	u8 Id;
//	u8 Len;         //数据长度
//	u8 Data[200];   //数据
//}Packet;

#define RF433SEND_MAX 3 

void IRorRF_Init(void);
void SendRf433One(void);
void SendRf433Zero(void);
void SendRf433Byte(u8 btye);
void Task_rf433(void);
void Send_rf433(void);
void Send_rf433_1(void);
void Rf433TxSocket(void);
void OnSlave(void);

u8 GetCrcCheck(u8 *p,u8 len);
void DisCode_Ir(u8 *p,u8 len);
void New_IR_Send(void);//装载码库数据,并发送红外码库数据
void ProcessIrStudyData(void);//学习数据处理
void SendIrStudy(IrStudy_DATA * stp_studyData);
void Send433Study(IrStudy_DATA * stp_studyData);
#endif
