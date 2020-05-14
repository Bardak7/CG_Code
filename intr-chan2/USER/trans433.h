#ifndef __TRANS433_H
#define __TRANS433_H

#include "stm8s.h"
#include "delay.h"
#include "stm8s_gpio.h"
#include "tim1.h"
#include "stdlib.h"


//#define UART


/**Device Type**/
#define Master 0x00  //����
//#define Rep    0x01  //�м�
#define SWITCH   0x02  //����
#define SW_Chan  0x02    //����·��3��ʾ3������

/**Cmd Type**/
#define  SW_ON     0x11 //���ؿ�
#define  ADD_SW    0x31 //�������͵���ӿ�������

/*EEPROM��ַ����**/

#define MasterAddr_Mark1   0x4004 //EEPROM��������ַ����ƥ���־λ
#define MasterAddr_Mark2   0x4005
#define MasterAddr_Beg     0x4000 //������ַ��ʼ��ַ

#define RemoteAddr_Mark1  0x400B //EEPROMң��������ƥ���־λ�ĵ�ַ
#define RemoteAddr_Mark2  0x400C  //���������־λ�Ƿ���ͱ�ʾ��ȷ
#define RemoteAddr_Beg     0x4020  //ң�ص�ַ�����EEPROM�е�ַ��ʼ��λ��

#define LOCAL_ADDR_BEG     0x4010

/**���������ط����� **/
#define SENDCOUNT_MAX  5

enum E_processRead_flag
{
   crcError,
   master,
   remote,
   notMasterOrRemote,
   getData
};

//433�Ľ���״̬����
enum E_rf433_state
{
	nowork, //û���ڽ�������
	inwork, //���ڽ����������յ�ǰ���ź�
	workok, //�������
};

//433���͵�����
typedef struct {
  u32 source_addr;
  u32 target_addr;
  u8  device_type;
  u8  cmd_type;
  u8  Id;
  u8  data_len;
  u8  data[10];
}Str_433_tx_data;
/**
  �˽ṹ������Ϊ���ռ������õ�ʱ��
  flag��Ա��־���붨ʱ�����Ƿ���� counter��Ա��Ϊ������
  ����Ϊ�����������ʱ��
     if(counter>0) counter--;
   flag���ڱ�־�Ƿ���Ҫ����
**/
typedef struct{
  bool sendread_flag;
  volatile u16 counter;
}rf433_flag;
//�˽ṹ���ڱ�־��״̬
typedef struct
{
//  rf433_flag rx_counter;  
  rf433_flag tx_counter;
  enum E_rf433_state data_sta;    //�������ڴ���ʲô״̬��
//  u8 busy;//:1;        //433��·æ״̬
//  u8 head;//:1;        //���յ�ͷ״̬
  u8 master_sta:1;  //������ַ�Ƿ�
  u8 remote_sta:1;  //ң�ص�ַ�Ƿ�
  u8 wait_master:1;  //�Ƿ�ȴ��������
  u8 wait_remote:1;  //�Ƿ�ȵ����ң����
  u8 ack:1;          //�Ƿ���յ���Ӧ����Ӧ��Ϊ0���յ�Ӧ��Ϊ1
//  u8 insend: 1;  
} Str_433;



//#define FS(n)  n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_3):GPIO_WriteLow(GPIOC,GPIO_PIN_3)  //433����
//
//#define DO (GPIOD->IDR&0x04)  

extern u8 Touch; ///����״̬λ
extern Str_433           Str433_flag;
extern Str_433_tx_data   Str433_tx;
extern u8 G_RX433_Data[70];
extern u32 G_RmeoteAddr;  //ң�����ĵ�ַ��¼
extern u8 G_Rf433Send_RandCnt;
extern u8 G_Rf433Send_RandCnt;
extern u32 G_Endian_LocalAddr;
extern u32 G_Endian_MasterAddr;
//extern u16 G_SendCount;


enum E_processRead_flag Process_433Read(); //��ȡ
void Process_433Send();     //�ֽڷ���
void Judge_433Send();
//bool CheckCrc();               //У��CRC
//void CalculateCrc(u8 *p,u8 len);//����CRC
void OnSlave(void);
#endif