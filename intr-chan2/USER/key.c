#include "key.h"
#include "stm8s_flash.h"
#include "uart.h"
#include "radio.h"
Str_KeyFlag StrKey_flag;
u16 G_LedCount=0;
extern u32 G_Endian_MasterAddr;
//FunctionalState G_LedPolarity; 

/**
  等待主机发送地址到开关来匹配
  @parm Touch由上面函数传递下来的开关状态

**/
void Get_MasterAddr()
{
 
//  u16 Led_count=0;
  u8 addr_flag=0;
  u8 i;
  u8 * addrTmp;
   FLASH_Unlock(FLASH_MEMTYPE_DATA);  //只要长按了5S
   FLASH_EraseByte(MasterAddr_Mark1); //清除主机地址的标志位
   IWDG->KR=0xAA;//喂狗
#ifdef UART  
   UART1_SendString("主机\r\n");
#endif 
   StrKey_flag.key_channel=0;
  while(1) 
  {IWDG->KR=0xAA;
    //LED闪烁提示
  if(G_LedCount==0){
    //GPIO_WriteReverse(GPIOA,GPIO_PIN_3);
    REVERSE_LED();
    G_LedCount=BLINK_TIME;
  }
   OnSlave(); //接收和处理
   if(addr_flag==0) //如果没有记录主机地址
    {
     if(Str433_flag.data_sta==workok)
      {
        Str_433_tx_data * recive_data;
        recive_data=(Str_433_tx_data * )G_RX433_Data; //把数组强制用结构体指针构造
    
        Str433_flag.data_sta=nowork;//清除标志位
 
//      if(CheckCrc()==TRUE)//如果检验成功
       {
        
        if(recive_data->cmd_type==ADD_SW)  //命令类型0x31表示添加开关
         {
#ifdef UART
          UART1_SendString("收到添加命令\r\n");
#endif
          //把收到的主机地址做记录
          FLASH_Unlock(FLASH_MEMTYPE_DATA);
          FLASH_ProgramWord(MasterAddr_Beg,recive_data->source_addr);//记录地址
          
          addrTmp=(u8 *)&recive_data->source_addr;
          for(i=0;i<4;i++){
           G_Endian_MasterAddr|=((u32)(*addrTmp++))<<(i*8); //大小端转换
          }
          
         Str433_tx.target_addr=recive_data->source_addr;//把主机给的地址添加到发送数组的目标地址中去
 
         Str433_tx.device_type=SWITCH; 
         Str433_tx.cmd_type=ADD_SW|0x80; //如果收到了需要返回数据的命令类型最高位置1
         
         Str433_tx.data_len=2;  //数据体2

         Str433_tx.data[0]=Touch;  //开关状态
         Str433_tx.data[1]=SW_Chan;
         
//         CalculateCrc((u8 *)(&Str433_tx),Str433_tx.data_len+12);  //计算校验
         Str433_flag.tx_counter.sendread_flag=TRUE;//立即发送一次
         Str433_flag.ack=1;  //发送后需要主机回复
//         Judge_433Send(); //判断是否可以发送
         addr_flag=1;
        }
       }
      }
    } 
    //已经记录好主机地址  向主机返回数据 并等待主机的确认
    else if(addr_flag==1) 
     {
       if(Str433_flag.data_sta==workok)
       {
         
         Str433_flag.data_sta=nowork;
         Str_433_tx_data * recive_data;
       recive_data=(Str_433_tx_data * )G_RX433_Data; //把数组强制用结构体指针构造

        
      //如果已经接收到过主机的广播且返回给主机相应的数据那么主机会对这个地址响应一个添加成功0x23命令
         if(recive_data->cmd_type==0x23)
          {
//               if(CheckCrc()) //如果地址正确且收到了主机的添加成功命令那么不需要再发送了
               {
                 Str433_flag.ack=0;//不需要再继续发送了
                 Str433_flag.master_sta=1; 
  #ifdef UART
                 UART1_SendString("已成功匹配完成\r\n");
  #endif
                 FLASH_Unlock(FLASH_MEMTYPE_DATA);
                 FLASH_ProgramByte(MasterAddr_Mark1,0xA9);  //EEPROM里做标记下次上电还知道已记录过地址
                 FLASH_ProgramByte(MasterAddr_Mark2,0x56);
                   Str433_flag.wait_master=0;
                 Cmt2300_GoStby();
//                 Cmt2300_SetResendTime(1);
                 Cmt2300_SetSynWord(G_Endian_LocalAddr);//0xFFFFFFFF
                 Cmt2300_GoSleep();
                 break;   //如果收到主机的0x23确认添加那么整个添加开关过程完成
               }
            }
        }
      }
   IWDG->KR=0xAA;
   Process_433Send();
   IWDG->KR=0xAA;
   if(StrKey_flag.key_channel & 0x07) //如果进入添加状态不想添加或者只是想清除这个已添加状态再按一下就可以退出
    {
      StrKey_flag.key_channel=0;//delay_ms(100);
      Str433_flag.ack=0;  //不需要再继续发送了。
      Str433_flag.master_sta=0; 
      Str433_flag.wait_master=0;
      break;
    }
    IWDG->KR=0xAA;
  }
}

///**
// 函数功能：获取遥控器地址
// 
//**/
//void Get_RemoteAddr(void)
//{
// u16 Led_count=0;
// u8 i;
// Str_433_tx_data * recive_data;
//   recive_data=(Str_433_tx_data * )G_RX433_Data; //把接收的数组强制转换成我们的数据体结构
// Str433_flag.remote_sta=0;
// FLASH_Unlock(FLASH_MEMTYPE_DATA);  //只要长按了10S
// FLASH_EraseByte(RemoteAddr_Mark1); //清除遥控地址的标志位
// for(i=0;i<4;i++){
//  FLASH_EraseByte(RemoteAddr_Beg+i);
// }
// UART1_SendString("遥控\r\n");
// StrKey_flag.key_channel=0;
// while(1)
// {
//   Led_count++;
//   if(Led_count>3000)
//    {LED1(1);delay_us(10);if(Led_count==6000)Led_count=0;}//闪烁指示
//   else {LED1(0);delay_us(10);}
//   if(Str433_flag.data_sta==workok)
//   {
//     
//     Str433_flag.data_sta=nowork;
//     if(CheckCrc())
//     {
//       G_RmeoteAddr=recive_data->source_addr;
//       FLASH_Unlock(FLASH_MEMTYPE_DATA);
//       FLASH_ProgramWord(RemoteAddr_Beg,G_RmeoteAddr);
//       FLASH_ProgramByte(RemoteAddr_Mark1,0xA9);  //EEPROM里做标记下次上电还知道已记录过地址
//       FLASH_ProgramByte(RemoteAddr_Mark2,0x56);
//       Str433_flag.remote_sta=1;
//       break;
////       for(i=0;i<4;i++)
////       FLASH_ProgramByte(RemoteAddr_Beg,Point_433->Read_Buff[i]);
////       break;
//     }
//     
//   }
//    //Process_433Send();
//   if(StrKey_flag.key_channel & 0x07){StrKey_flag.key_channel=0;delay_ms(100); break;} //如果再次按键那么就退出遥控器添加
//   IWDG->KR=0xAA;
// }
//}

