#ifndef __TRANS433_H
#define __TRANS433_H

#include "stm8s.h"
#include "delay.h"
#include "stm8s_gpio.h"
#include "tim1.h"
#include "stdlib.h"


//#define UART


/**Device Type**/
#define Master 0x00  //主机
//#define Rep    0x01  //中继
#define SWITCH   0x02  //开关
#define SW_Chan  0x02    //开关路数3表示3个开关

/**Cmd Type**/
#define  SW_ON     0x11 //开关开
#define  ADD_SW    0x31 //主机发送的添加开关命令

/*EEPROM地址含义**/

#define MasterAddr_Mark1   0x4004 //EEPROM中主机地址有无匹配标志位
#define MasterAddr_Mark2   0x4005
#define MasterAddr_Beg     0x4000 //主机地址开始地址

#define RemoteAddr_Mark1  0x400B //EEPROM遥控器有无匹配标志位的地址
#define RemoteAddr_Mark2  0x400C  //如果两个标志位是反码就表示正确
#define RemoteAddr_Beg     0x4020  //遥控地址存放在EEPROM中地址开始的位置

#define LOCAL_ADDR_BEG     0x4010

/**定义最大的重发次数 **/
#define SENDCOUNT_MAX  5

enum E_processRead_flag
{
   crcError,
   master,
   remote,
   notMasterOrRemote,
   getData
};

//433的接收状态定义
enum E_rf433_state
{
	nowork, //没有在接受数据
	inwork, //正在接受数据已收到前导信号
	workok, //接收完成
};

//433发送的数据
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
  此结构体在做为接收计数器用的时候
  flag成员标志进入定时器后是否计数 counter成员作为计数器
  当做为发射计数器的时候
     if(counter>0) counter--;
   flag用于标志是否需要发送
**/
typedef struct{
  bool sendread_flag;
  volatile u16 counter;
}rf433_flag;
//此结构用于标志各状态
typedef struct
{
//  rf433_flag rx_counter;  
  rf433_flag tx_counter;
  enum E_rf433_state data_sta;    //数据现在处于什么状态。
//  u8 busy;//:1;        //433线路忙状态
//  u8 head;//:1;        //已收到头状态
  u8 master_sta:1;  //主机地址是否
  u8 remote_sta:1;  //遥控地址是否
  u8 wait_master:1;  //是否等待添加主机
  u8 wait_remote:1;  //是否等等添加遥控器
  u8 ack:1;          //是否接收到了应答，已应答为0无收到应答为1
//  u8 insend: 1;  
} Str_433;



//#define FS(n)  n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_3):GPIO_WriteLow(GPIOC,GPIO_PIN_3)  //433发射
//
//#define DO (GPIOD->IDR&0x04)  

extern u8 Touch; ///开关状态位
extern Str_433           Str433_flag;
extern Str_433_tx_data   Str433_tx;
extern u8 G_RX433_Data[70];
extern u32 G_RmeoteAddr;  //遥控器的地址记录
extern u8 G_Rf433Send_RandCnt;
extern u8 G_Rf433Send_RandCnt;
extern u32 G_Endian_LocalAddr;
extern u32 G_Endian_MasterAddr;
//extern u16 G_SendCount;


enum E_processRead_flag Process_433Read(); //读取
void Process_433Send();     //字节发送
void Judge_433Send();
//bool CheckCrc();               //校验CRC
//void CalculateCrc(u8 *p,u8 len);//计算CRC
void OnSlave(void);
#endif