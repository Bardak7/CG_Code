#ifndef __IRorRF_H
#define __IRorRF_H	

#include "typedefs.h"
#include "sys.h"

//typedef struct 
//{
//	u32 SoucreAddr; //Դ��ַ
//	u32 DistAddr;   //Ŀ���ַ
//  u8 Type;        //�豸����
//	u8 Cmd;         //��������  0x00-����  0x01-�м� 0x02-����
//	u8 Id;
//	u8 Len;         //���ݳ���
//	u8 Data[200];   //����
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
void New_IR_Send(void);//װ���������,�����ͺ����������
void ProcessIrStudyData(void);//ѧϰ���ݴ���
void SendIrStudy(IrStudy_DATA * stp_studyData);
void Send433Study(IrStudy_DATA * stp_studyData);
#endif
