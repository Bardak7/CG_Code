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

//IO�ڵ�ַӳ��
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
 
//IO�ڲ���,ֻ�Ե�һ��IO��!
//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //��� 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //���� 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //��� 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //���� 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //��� 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //���� 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //��� 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //���� 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //��� 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //����

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //��� 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //����

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //��� 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //����

#define Key PBin(13)
#define IrOut	PBout(12)
#define Rf433Out PBout(5)
#define Rf433In PBin(6)

//��� PA1   ���� PA2
#define ACT_LED_EN PAout(1)=0
#define ACT_LED_DIS	PAout(1)=1

#define MCU_LED_EN PAout(2)=0
#define MCU_LED_DIS	PAout(2)=1  
//=============================================
#define On  1
#define Off 0

enum rev_func
{
	nowork, //û���ڽ�������
	inwork, //���ڽ�������
	workok, //�������
};

struct SysTim   //ʱ��ṹ
{
	u8 date;      
	u8 hr;        //Сʱ
	u8 min;       //��
	u8 sec;       //��
};

struct AdcAct
{
	u32 AdcVal;
	u8  AdcOk;
};

struct LedAct
{
	u8 LedCnt; //���ƽ����ʱ��
	u8 LedStat;//��ƽ
	u8 LedNum; //��˸����
};

struct IsIrInt
{
  u8 flag;
  u16 cnt;
};

typedef struct 
{
	u32 SoucreAddr;  //Դ��ַ
	u32 DistAddr;    //Ŀ���ַ
  u8 Type;        //�豸����
	u8 Cmd;         //��������  0x00-����  0x01-�м� 0x02-����
	u8 Id;
	u8 Len;         //���ݳ���
	u8 Data[255];    //����
}IrOrRf_DATA;

typedef struct IrStudy//����ѧϰ�����ݽṹ-28
{
	u16 LibCHK;  //����飬���ӿͻ��˷������Ŀ�����������⻹������ѧϰ���� �̶�Ϊ0x5AA5;
	u16 HeadHCnt;//ͷ��ߵ�ƽʱ�ӣ�n*13.16us
	u16 HeadLCnt;//ͷ��͵�ƽʱ�ӣ�n*13.16us
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
	struct SysTim tim;  //ϵͳʱ��
	u32 MasterAddr;     //������ַ
	u32 SysAddr;        //������ַ
	u8 IrTxH_flag:1;
	u8 Rf433Send_flag:1;
	u8 bWasAdd:1;//�ֻ��ѱ���ӱ�־
	u8 channel;//·��
	u8 key_press;//״̬
	u16 Tempt;
}SysStruct;

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
#define FLASH_SAVE_ADDR0 0x1FFFF7E8		
#define FLASH_SAVE_ADDR1 0x08019000	//������ַ��Ŵ����ִ��0xF0,0x0F��4�ֽڵ�ַ��0xF0,0x0F
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
