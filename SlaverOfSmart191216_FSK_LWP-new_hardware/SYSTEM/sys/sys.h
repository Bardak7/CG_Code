#ifndef __SYS_H
#define __SYS_H	
#include "stm32f10x.h"
#include "stm32f10x_it.h" 
#include "stm32f10x_usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "delay.h"
//#include "timer.h"
#include "led.h"
#include "key.h"
#include "adc.h"
#include "wdg.h"
#include "exti.h"
//#include "IRorRF.h"

#include "typedefs.h"

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

#define Key PBin(13)
#define IrOut	PBout(12)
#define Rf433Out PBout(5)
#define Rf433In PBin(6)

//红灯 PA1   蓝灯 PA2
#define ACT_LED_EN PAout(1)=0
#define ACT_LED_DIS	PAout(1)=1

#define MCU_LED_EN PAout(2)=0
#define MCU_LED_DIS	PAout(2)=1  
//=============================================
#define On  1
#define Off 0

enum rev_func
{
	nowork, //没有在接受数据
	inwork, //正在接受数据
	workok, //接收完成
};

struct SysTim   //时间结构
{
	u8 date;      
	u8 hr;        //小时
	u8 min;       //分
	u8 sec;       //秒
};

struct AdcAct
{
	u32 AdcVal;
	u8  AdcOk;
};

struct LedAct
{
	u8 LedCnt; //点电平持续时间
	u8 LedStat;//电平
	u8 LedNum; //闪烁次数
};

struct IsIrInt
{
  u8 flag;
  u16 cnt;
};

typedef struct 
{
	u32 SoucreAddr;  //源地址
	u32 DistAddr;    //目标地址
  u8 Type;        //设备类型
	u8 Cmd;         //命令类型  0x00-主机  0x01-中继 0x02-开关
	u8 Id;
	u8 Len;         //数据长度
	u8 Data[255];    //数据
}IrOrRf_DATA;

typedef struct IrStudy//红外学习体数据结构-28
{
	u16 LibCHK;  //码库检查，检查从客户端发下来的控制数据是码库还是我们学习的码 固定为0x5AA5;
	u16 HeadHCnt;//头码高电平时延，n*13.16us
	u16 HeadLCnt;//头码低电平时延，n*13.16us
	u16 EndHCnt;
	u16 EndLCnt;
	u16 Data1HCnt;  
	u16 Data1LCnt;
	u16 Data0HCnt;
	u16 Data0LCnt;
	u8 Type;  //ir-1,433-2,315-3
	u8 Len;
	u8 Data[50];
}IrStudy_DATA;

typedef struct //58
{
	struct SysTim tim;  //系统时间
	u32 MasterAddr;     //主机地址
	u32 SysAddr;        //本机地址
	u8 IrTxH_flag:1;
	u8 Rf433Send_flag:1;
	u8 bWasAdd:1;//分机已被添加标志
	u8 channel;//路数
	u8 key_press;//状态
	u16 Tempt;
}SysStruct;

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
#define FLASH_SAVE_ADDR0 0x1FFFF7E8		
#define FLASH_SAVE_ADDR1 0x08019000	//主机地址存放处，现存放0xF0,0x0F，4字节地址，0xF0,0x0F
#define EE_ENABLE
//=========================================================================
#define AddSendTime 75  //75*20ms=1.5s
void NVIC_Configuration(void);
void TaskTime(void);
void DataInit(void);
void AddSendData(void);
void INTX_DISABLE(void);
void INTX_ENABLE(void);
#endif
