#include "key.h"
#include "stm8s_flash.h"
#include "uart.h"
#include "radio.h"
Str_KeyFlag StrKey_flag;
u16 G_LedCount=0;
extern u32 G_Endian_MasterAddr;
//FunctionalState G_LedPolarity; 

/**
  �ȴ��������͵�ַ��������ƥ��
  @parm Touch�����溯�����������Ŀ���״̬

**/
void Get_MasterAddr()
{
 
//  u16 Led_count=0;
  u8 addr_flag=0;
  u8 i;
  u8 * addrTmp;
   FLASH_Unlock(FLASH_MEMTYPE_DATA);  //ֻҪ������5S
   FLASH_EraseByte(MasterAddr_Mark1); //���������ַ�ı�־λ
   IWDG->KR=0xAA;//ι��
#ifdef UART  
   UART1_SendString("����\r\n");
#endif 
   StrKey_flag.key_channel=0;
  while(1) 
  {IWDG->KR=0xAA;
    //LED��˸��ʾ
  if(G_LedCount==0){
    //GPIO_WriteReverse(GPIOA,GPIO_PIN_3);
    REVERSE_LED();
    G_LedCount=BLINK_TIME;
  }
   OnSlave(); //���պʹ���
   if(addr_flag==0) //���û�м�¼������ַ
    {
     if(Str433_flag.data_sta==workok)
      {
        Str_433_tx_data * recive_data;
        recive_data=(Str_433_tx_data * )G_RX433_Data; //������ǿ���ýṹ��ָ�빹��
    
        Str433_flag.data_sta=nowork;//�����־λ
 
//      if(CheckCrc()==TRUE)//�������ɹ�
       {
        
        if(recive_data->cmd_type==ADD_SW)  //��������0x31��ʾ��ӿ���
         {
#ifdef UART
          UART1_SendString("�յ��������\r\n");
#endif
          //���յ���������ַ����¼
          FLASH_Unlock(FLASH_MEMTYPE_DATA);
          FLASH_ProgramWord(MasterAddr_Beg,recive_data->source_addr);//��¼��ַ
          
          addrTmp=(u8 *)&recive_data->source_addr;
          for(i=0;i<4;i++){
           G_Endian_MasterAddr|=((u32)(*addrTmp++))<<(i*8); //��С��ת��
          }
          
         Str433_tx.target_addr=recive_data->source_addr;//���������ĵ�ַ��ӵ����������Ŀ���ַ��ȥ
 
         Str433_tx.device_type=SWITCH; 
         Str433_tx.cmd_type=ADD_SW|0x80; //����յ�����Ҫ�������ݵ������������λ��1
         
         Str433_tx.data_len=2;  //������2

         Str433_tx.data[0]=Touch;  //����״̬
         Str433_tx.data[1]=SW_Chan;
         
//         CalculateCrc((u8 *)(&Str433_tx),Str433_tx.data_len+12);  //����У��
         Str433_flag.tx_counter.sendread_flag=TRUE;//��������һ��
         Str433_flag.ack=1;  //���ͺ���Ҫ�����ظ�
//         Judge_433Send(); //�ж��Ƿ���Է���
         addr_flag=1;
        }
       }
      }
    } 
    //�Ѿ���¼��������ַ  �������������� ���ȴ�������ȷ��
    else if(addr_flag==1) 
     {
       if(Str433_flag.data_sta==workok)
       {
         
         Str433_flag.data_sta=nowork;
         Str_433_tx_data * recive_data;
       recive_data=(Str_433_tx_data * )G_RX433_Data; //������ǿ���ýṹ��ָ�빹��

        
      //����Ѿ����յ��������Ĺ㲥�ҷ��ظ�������Ӧ��������ô������������ַ��Ӧһ����ӳɹ�0x23����
         if(recive_data->cmd_type==0x23)
          {
//               if(CheckCrc()) //�����ַ��ȷ���յ�����������ӳɹ�������ô����Ҫ�ٷ�����
               {
                 Str433_flag.ack=0;//����Ҫ�ټ���������
                 Str433_flag.master_sta=1; 
  #ifdef UART
                 UART1_SendString("�ѳɹ�ƥ�����\r\n");
  #endif
                 FLASH_Unlock(FLASH_MEMTYPE_DATA);
                 FLASH_ProgramByte(MasterAddr_Mark1,0xA9);  //EEPROM��������´��ϵ绹֪���Ѽ�¼����ַ
                 FLASH_ProgramByte(MasterAddr_Mark2,0x56);
                   Str433_flag.wait_master=0;
                 Cmt2300_GoStby();
//                 Cmt2300_SetResendTime(1);
                 Cmt2300_SetSynWord(G_Endian_LocalAddr);//0xFFFFFFFF
                 Cmt2300_GoSleep();
                 break;   //����յ�������0x23ȷ�������ô������ӿ��ع������
               }
            }
        }
      }
   IWDG->KR=0xAA;
   Process_433Send();
   IWDG->KR=0xAA;
   if(StrKey_flag.key_channel & 0x07) //����������״̬������ӻ���ֻ���������������״̬�ٰ�һ�¾Ϳ����˳�
    {
      StrKey_flag.key_channel=0;//delay_ms(100);
      Str433_flag.ack=0;  //����Ҫ�ټ��������ˡ�
      Str433_flag.master_sta=0; 
      Str433_flag.wait_master=0;
      break;
    }
    IWDG->KR=0xAA;
  }
}

///**
// �������ܣ���ȡң������ַ
// 
//**/
//void Get_RemoteAddr(void)
//{
// u16 Led_count=0;
// u8 i;
// Str_433_tx_data * recive_data;
//   recive_data=(Str_433_tx_data * )G_RX433_Data; //�ѽ��յ�����ǿ��ת�������ǵ�������ṹ
// Str433_flag.remote_sta=0;
// FLASH_Unlock(FLASH_MEMTYPE_DATA);  //ֻҪ������10S
// FLASH_EraseByte(RemoteAddr_Mark1); //���ң�ص�ַ�ı�־λ
// for(i=0;i<4;i++){
//  FLASH_EraseByte(RemoteAddr_Beg+i);
// }
// UART1_SendString("ң��\r\n");
// StrKey_flag.key_channel=0;
// while(1)
// {
//   Led_count++;
//   if(Led_count>3000)
//    {LED1(1);delay_us(10);if(Led_count==6000)Led_count=0;}//��˸ָʾ
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
//       FLASH_ProgramByte(RemoteAddr_Mark1,0xA9);  //EEPROM��������´��ϵ绹֪���Ѽ�¼����ַ
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
//   if(StrKey_flag.key_channel & 0x07){StrKey_flag.key_channel=0;delay_ms(100); break;} //����ٴΰ�����ô���˳�ң�������
//   IWDG->KR=0xAA;
// }
//}

