#ifndef __KEY_H
#define __KEY_H

#include "trans433.h"

#define LONG_PRESS      4     //�����ĳ���ʱ��
#define VERLONG_PRESS   8     //�ǳ����İ���ʱ��
#define BLINK_TIME      1    //��˸ʱ�䣬�������ж���һ��

/**�������Ŷ���**/
#define TOUCH2 (GPIOD->IDR&0x04)  //GPIOD_PIN_2
#define TOUCH1 (GPIOD->IDR&0x08)  //GPIOD_PIN_3
//#define Touch3 (GPIOA->IDR&0x08)

/**LED�ĺ궨��**/
//#define LED3(n) n&1?GPIO_WriteLow(GPIOB,GPIO_PIN_5):GPIO_WriteHigh(GPIOB,GPIO_PIN_5)
#define LED2(n) n&1?GPIO_WriteLow(GPIOB,GPIO_PIN_5):GPIO_WriteHigh(GPIOB,GPIO_PIN_5)
#define LED1(n) n&1?GPIO_WriteLow(GPIOD,GPIO_PIN_6):GPIO_WriteHigh(GPIOD,GPIO_PIN_6)


/**����ĺ궨��**/
#define OUT1(n) n&1?GPIO_WriteLow(GPIOD,GPIO_PIN_5):GPIO_WriteHigh(GPIOD,GPIO_PIN_5)  
#define OUT2(n) n&1?GPIO_WriteLow(GPIOD,GPIO_PIN_4):GPIO_WriteHigh(GPIOD,GPIO_PIN_4)

/**��תLED�궨��**/
#define REVERSE_LED() {GPIO_WriteReverse(GPIOB,GPIO_PIN_5);GPIO_WriteReverse(GPIOD,GPIO_PIN_6);}

typedef struct{
  u8 key_channel;
  u8 key_countFlag;
  u16 key_count;
}Str_KeyFlag;

extern u16 G_LedCount;
//extern FunctionalState G_LedPolarity; 
extern Str_KeyFlag StrKey_flag;

void Get_RemoteAddr(void);
void KeyScan();
void Get_MasterAddr();
#endif