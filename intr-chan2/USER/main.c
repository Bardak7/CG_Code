

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "sysclock.h"
#include "stm8s_exti.h"
#include "uart.h"
#include "stm8s_gpio.h"
#include "delay.h"
#include "stm8s_flash.h"
#include "trans433.h"
#include "tim1.h"
#include "stm8s_itc.h"
#include "key.h"
#include "radio.h"
/**Touch定义**/
//#define T_MODE(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_5):GPIO_WriteLow(GPIOC,GPIO_PIN_5)//宏定义触摸模式:高锁存低非锁存
//#define T_SLPEN(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_5):GPIO_WriteLow(GPIOC,GPIO_PIN_5) //高睡眠

  u8 Touch;//开关的状态
  u32 G_Endian_MasterAddr;
  u32 G_Endian_LocalAddr;
  Str_433           Str433_flag;
  Str_433_tx_data   Str433_tx;
  u8  G_RX433_Data[70];
  u32 G_RmeoteAddr;//遥控器地址
/**IO口的初始化**/
void GPIO_init(void)
{

  
  /**LED引脚初始化**/
   GPIO_Init(GPIOD,GPIO_PIN_6,GPIO_MODE_OUT_PP_LOW_FAST); //GPIOD 6用于LED1的控制
   GPIO_Init(GPIOB,GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_FAST); //GPOOB 5用于LED2的控制
  
  /**输出到外部的引脚初始化**/
  GPIO_Init(GPIOD,GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_FAST);  //GPIOD 5 输出控制驱动板CONTR1
  GPIO_Init(GPIOD,GPIO_PIN_4,GPIO_MODE_OUT_PP_LOW_FAST);  //GPIOD 4 输出控制驱动板CONTR2       
  

  /**Touch引脚初始化**/
  GPIO_Init(GPIOD,GPIO_PIN_2,GPIO_MODE_IN_PU_IT);  //GPIOD 2接触摸芯片的输出 MCU的输入 Touch1
  GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_IN_PU_IT);  //GPIOD 3接触摸芯片的输出 MCU的输入 Touch2
  
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD,EXTI_SENSITIVITY_RISE_FALL); //设置为下降上升沿触发
  ITC_SetSoftwarePriority(ITC_IRQ_PORTD,ITC_PRIORITYLEVEL_1);
}

/**
*初始化数据
**/
void DATA_Init(void)
{
     u8 i;
    u8 addr1;
    u8 addr2;
    FLASH_DeInit();           //恢复默认的FLASH设置
    addr1=FLASH_ReadByte(MasterAddr_Mark1);
    addr2=~FLASH_ReadByte(MasterAddr_Mark2);
         
       if(addr1==addr2)//读主机匹配标志位
        {
         Str433_flag.master_sta=1;  //标记已得到过主机地址按键后需要发送报告
         FLASH_DeInit();
//         Temp_addr=FLASH_ReadByte(MasterAddr_Beg);
//         Temp_addr<<=24;
//         Str433_tx.source_addr=Temp_addr;
         for(i=0;i<4;i++)
          { 
            addr1=FLASH_ReadByte(MasterAddr_Beg+i);
//            Str433_tx.target_addr|=(((u32)FLASH_ReadByte(MasterAddr_Beg+i))<<((3-i)*8)); //从EEPROM提取主机地址并放到发送结构体中
//            G_Endian_MasterAddr  |=(((u32)FLASH_ReadByte(MasterAddr_Beg+i))<<(i*8));//小端取地址
            Str433_tx.target_addr<<=8;
            Str433_tx.target_addr|=addr1;
            G_Endian_MasterAddr|=((u32)addr1<<i*8);
           }
#ifdef UART          
         UART1_SendString("\r\n已录入过主机地址\r\n");
#endif
        }
       else 
        {
#ifdef UART         
         UART1_SendString("\r\n未录入过主机地址\r\n");
#endif   
        } 
       
       addr1=FLASH_ReadByte(RemoteAddr_Mark1);
       addr2=~FLASH_ReadByte(RemoteAddr_Mark2);
       if(addr1==addr2)//读遥控匹配标志位 
        {
          Str433_flag.remote_sta=1;
         for(i=0;i<4;i++)
          { 
            G_RmeoteAddr|=(((u32)FLASH_ReadByte(RemoteAddr_Beg+i))<<((3-i)*8)); //从EEPROM提取主机地址并放到发送结构体中
           } 
#ifdef UART          
         UART1_SendString("已录入过遥控器地址\r\n");
#endif
        }
       else 
       {
#ifdef UART         
         UART1_SendString("未录入过遥控器地址\r\n");
#endif    
       }//*
       
       //提取本地地址
       Str433_tx.source_addr=0;
       FLASH_DeInit();
for(i=0;i<4;i++)
          { 
            addr1=FLASH_ReadByte(LOCAL_ADDR_BEG+i);
//            Str433_tx.source_addr|=(((u32)FLASH_ReadByte(LOCAL_ADDR_BEG+i))<<((3-i)*8));//提取本地地址由烧录器滚动烧录。
//            G_Endian_LocalAddr|=(((u32)FLASH_ReadByte(LOCAL_ADDR_BEG+i))<<(i*8));
            Str433_tx.source_addr<<=8;
            Str433_tx.source_addr|=addr1; 
            G_Endian_LocalAddr|=((u32)addr1<<(i*8));
          }
//Str433_tx.source_addr=0x10101111;
#ifdef UART 
  UART1_SendString("本地：\r\n");
for(i=0;i<4;i++){
           
            UART1_SendByte(*((u8*)(&Str433_tx.source_addr)+i));
}
#endif
   Str433_flag.ack=0;//避免开机发送
}


 int main(void)
{      
 
     /*                32位源地址，        32位目标，         设备类型，命令类型，数据长度，数据体，   检验字*/
     //u8 data_array[]={0x01,0x01,0x01,0x01,  0x10,0x10,0x10,0x10,     0x02,     0x21,       0x02,    0x03,0x01,   0x7B};
	/* 系统时钟初始化 */
	SystemClock_Init();
	delay_init(16);  //延时函数的初始化
        GPIO_init();   //初始化相应的GPIO口
//	TIM1_Init();   //定时器1初始化 用于读取和发送22us
        TIM2_Init();   //10MS进一次中断用于发送
	     
#ifdef UART        
	Uart1_Init();        //初始化UART1
        UART1_SendString("System Init Finish!\n"); //串中发送数据
#endif 
        enableInterrupts();   //开中断   
 //         T_MODE(0);   //触摸芯片模式0为非锁存  
         
        IWDG->KR=0xCC;   //0xCC启动看门狗
        IWDG->KR=0x55; //打开保护        
        IWDG->RLR=0xFF;   //设置独立看门狗的重载值为0XFF
        IWDG->PR=0x06;    //256分频
        IWDG->KR=0xAA;
        DATA_Init();
        RF_Init_FSK();//无线模块初始化
        while(FALSE==Cmt2300_IsExist())
        {
          LED1(0);
          delay_ms(100);
          LED1(1);
          delay_ms(100);
        }    
        RF_StartRx(G_RX433_Data,64,0xFFFFFFFF);
       if(Str433_flag.master_sta==1){
        Str433_tx.cmd_type=0x92; //命令类型
        Str433_tx.data_len=2;
        Str433_tx.device_type=SWITCH;
        Str433_tx.data[Str433_tx.data_len]=0;//校验位
        Str433_tx.data[0]=Touch;
        Str433_tx.data[1]=SW_Chan;  
//        Cmt2300_GoStby();
//        Cmt2300_SetSynWord(G_Endian_MasterAddr);//sync改为发送给主机
//        Cmt2300_GoSleep();
//          CalculateCrc((u8 *)(&Str433_tx),Str433_tx.data_len+12);
        Str433_flag.ack=1;  //发送后需要主机回复
      }
      
while(1)
 { 

   IWDG->KR=0xAA;//喂狗
   

   if(Str433_flag.wait_master==1) { 
     
    Cmt2300_GoStby();
 //    Cmt2300_SetResendTime(1);
     Cmt2300_SetSynWord(0xFFFFFFFF);
     Cmt2300_SetResendTime(30);
     Cmt2300_GoSleep();
     Cmt2300_GoRx();
     Get_MasterAddr();
     Cmt2300_GoRx();
   }
   //else if(Str433_flag.==1){Str433_flag.wait_remote=0;Get_RemoteAddr();}

/**读取处理**/
 //if((Str433_flag.master_sta==1)||(Str433_flag.remote_sta==1))//当和主机匹配成功或遥控器匹配后才有接收的意义；
 {
//   enum E_processRead_flag recive_result;
   
   OnSlave();
   
   Process_433Read();
   Process_433Send();
 
#ifdef UART    
   switch(recive_result){
   case master :UART1_SendString("master\r\n");
     break;
   case remote :UART1_SendString("remote\r\n");
     break;
   case crcError :UART1_SendString("crcError\r\n");
     break;
   case notMasterOrRemote :UART1_SendString("notMasterOrRemote\r\n");
     break;
   default:
       break;
   }
#endif   
   IWDG->KR=0xAA;//喂狗
    
 }   

/**控制处理**/
 if(StrKey_flag.key_countFlag==0 && (Str433_flag.data_sta==nowork )){  //如果按键正在进行保持控制不做改变直到按键结束
  switch(Touch)
    {
      case 0x00: LED1(0);LED2(0);OUT1(1);OUT2(1);break;//OUT1(0);OUT2(0);
      case 0x01: LED1(1);LED2(0);OUT1(0);OUT2(1);break;// OUT1(1);OUT2(0);delay_ms(500);
      case 0x02: LED1(0);LED2(1);OUT1(1);OUT2(0);break;// OUT1(1);OUT2(0);delay_ms(500);
      case 0x03: LED1(1);LED2(1);OUT1(0);OUT2(0);break;// OUT1(1);OUT2(0);delay_ms(500);

    default: //StrKey_flag.key_channel=0;
        break;
    } //*/
   }   
  }  
}































#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/******************* (C) COPYRIGHT EW工作室 *****END OF FILE******************/
