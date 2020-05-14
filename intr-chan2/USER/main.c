

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
/**Touch����**/
//#define T_MODE(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_5):GPIO_WriteLow(GPIOC,GPIO_PIN_5)//�궨�崥��ģʽ:������ͷ�����
//#define T_SLPEN(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_5):GPIO_WriteLow(GPIOC,GPIO_PIN_5) //��˯��

  u8 Touch;//���ص�״̬
  u32 G_Endian_MasterAddr;
  u32 G_Endian_LocalAddr;
  Str_433           Str433_flag;
  Str_433_tx_data   Str433_tx;
  u8  G_RX433_Data[70];
  u32 G_RmeoteAddr;//ң������ַ
/**IO�ڵĳ�ʼ��**/
void GPIO_init(void)
{

  
  /**LED���ų�ʼ��**/
   GPIO_Init(GPIOD,GPIO_PIN_6,GPIO_MODE_OUT_PP_LOW_FAST); //GPIOD 6����LED1�Ŀ���
   GPIO_Init(GPIOB,GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_FAST); //GPOOB 5����LED2�Ŀ���
  
  /**������ⲿ�����ų�ʼ��**/
  GPIO_Init(GPIOD,GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_FAST);  //GPIOD 5 �������������CONTR1
  GPIO_Init(GPIOD,GPIO_PIN_4,GPIO_MODE_OUT_PP_LOW_FAST);  //GPIOD 4 �������������CONTR2       
  

  /**Touch���ų�ʼ��**/
  GPIO_Init(GPIOD,GPIO_PIN_2,GPIO_MODE_IN_PU_IT);  //GPIOD 2�Ӵ���оƬ����� MCU������ Touch1
  GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_IN_PU_IT);  //GPIOD 3�Ӵ���оƬ����� MCU������ Touch2
  
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD,EXTI_SENSITIVITY_RISE_FALL); //����Ϊ�½������ش���
  ITC_SetSoftwarePriority(ITC_IRQ_PORTD,ITC_PRIORITYLEVEL_1);
}

/**
*��ʼ������
**/
void DATA_Init(void)
{
     u8 i;
    u8 addr1;
    u8 addr2;
    FLASH_DeInit();           //�ָ�Ĭ�ϵ�FLASH����
    addr1=FLASH_ReadByte(MasterAddr_Mark1);
    addr2=~FLASH_ReadByte(MasterAddr_Mark2);
         
       if(addr1==addr2)//������ƥ���־λ
        {
         Str433_flag.master_sta=1;  //����ѵõ���������ַ��������Ҫ���ͱ���
         FLASH_DeInit();
//         Temp_addr=FLASH_ReadByte(MasterAddr_Beg);
//         Temp_addr<<=24;
//         Str433_tx.source_addr=Temp_addr;
         for(i=0;i<4;i++)
          { 
            addr1=FLASH_ReadByte(MasterAddr_Beg+i);
//            Str433_tx.target_addr|=(((u32)FLASH_ReadByte(MasterAddr_Beg+i))<<((3-i)*8)); //��EEPROM��ȡ������ַ���ŵ����ͽṹ����
//            G_Endian_MasterAddr  |=(((u32)FLASH_ReadByte(MasterAddr_Beg+i))<<(i*8));//С��ȡ��ַ
            Str433_tx.target_addr<<=8;
            Str433_tx.target_addr|=addr1;
            G_Endian_MasterAddr|=((u32)addr1<<i*8);
           }
#ifdef UART          
         UART1_SendString("\r\n��¼���������ַ\r\n");
#endif
        }
       else 
        {
#ifdef UART         
         UART1_SendString("\r\nδ¼���������ַ\r\n");
#endif   
        } 
       
       addr1=FLASH_ReadByte(RemoteAddr_Mark1);
       addr2=~FLASH_ReadByte(RemoteAddr_Mark2);
       if(addr1==addr2)//��ң��ƥ���־λ 
        {
          Str433_flag.remote_sta=1;
         for(i=0;i<4;i++)
          { 
            G_RmeoteAddr|=(((u32)FLASH_ReadByte(RemoteAddr_Beg+i))<<((3-i)*8)); //��EEPROM��ȡ������ַ���ŵ����ͽṹ����
           } 
#ifdef UART          
         UART1_SendString("��¼���ң������ַ\r\n");
#endif
        }
       else 
       {
#ifdef UART         
         UART1_SendString("δ¼���ң������ַ\r\n");
#endif    
       }//*
       
       //��ȡ���ص�ַ
       Str433_tx.source_addr=0;
       FLASH_DeInit();
for(i=0;i<4;i++)
          { 
            addr1=FLASH_ReadByte(LOCAL_ADDR_BEG+i);
//            Str433_tx.source_addr|=(((u32)FLASH_ReadByte(LOCAL_ADDR_BEG+i))<<((3-i)*8));//��ȡ���ص�ַ����¼��������¼��
//            G_Endian_LocalAddr|=(((u32)FLASH_ReadByte(LOCAL_ADDR_BEG+i))<<(i*8));
            Str433_tx.source_addr<<=8;
            Str433_tx.source_addr|=addr1; 
            G_Endian_LocalAddr|=((u32)addr1<<(i*8));
          }
//Str433_tx.source_addr=0x10101111;
#ifdef UART 
  UART1_SendString("���أ�\r\n");
for(i=0;i<4;i++){
           
            UART1_SendByte(*((u8*)(&Str433_tx.source_addr)+i));
}
#endif
   Str433_flag.ack=0;//���⿪������
}


 int main(void)
{      
 
     /*                32λԴ��ַ��        32λĿ�꣬         �豸���ͣ��������ͣ����ݳ��ȣ������壬   ������*/
     //u8 data_array[]={0x01,0x01,0x01,0x01,  0x10,0x10,0x10,0x10,     0x02,     0x21,       0x02,    0x03,0x01,   0x7B};
	/* ϵͳʱ�ӳ�ʼ�� */
	SystemClock_Init();
	delay_init(16);  //��ʱ�����ĳ�ʼ��
        GPIO_init();   //��ʼ����Ӧ��GPIO��
//	TIM1_Init();   //��ʱ��1��ʼ�� ���ڶ�ȡ�ͷ���22us
        TIM2_Init();   //10MS��һ���ж����ڷ���
	     
#ifdef UART        
	Uart1_Init();        //��ʼ��UART1
        UART1_SendString("System Init Finish!\n"); //���з�������
#endif 
        enableInterrupts();   //���ж�   
 //         T_MODE(0);   //����оƬģʽ0Ϊ������  
         
        IWDG->KR=0xCC;   //0xCC�������Ź�
        IWDG->KR=0x55; //�򿪱���        
        IWDG->RLR=0xFF;   //���ö������Ź�������ֵΪ0XFF
        IWDG->PR=0x06;    //256��Ƶ
        IWDG->KR=0xAA;
        DATA_Init();
        RF_Init_FSK();//����ģ���ʼ��
        while(FALSE==Cmt2300_IsExist())
        {
          LED1(0);
          delay_ms(100);
          LED1(1);
          delay_ms(100);
        }    
        RF_StartRx(G_RX433_Data,64,0xFFFFFFFF);
       if(Str433_flag.master_sta==1){
        Str433_tx.cmd_type=0x92; //��������
        Str433_tx.data_len=2;
        Str433_tx.device_type=SWITCH;
        Str433_tx.data[Str433_tx.data_len]=0;//У��λ
        Str433_tx.data[0]=Touch;
        Str433_tx.data[1]=SW_Chan;  
//        Cmt2300_GoStby();
//        Cmt2300_SetSynWord(G_Endian_MasterAddr);//sync��Ϊ���͸�����
//        Cmt2300_GoSleep();
//          CalculateCrc((u8 *)(&Str433_tx),Str433_tx.data_len+12);
        Str433_flag.ack=1;  //���ͺ���Ҫ�����ظ�
      }
      
while(1)
 { 

   IWDG->KR=0xAA;//ι��
   

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

/**��ȡ����**/
 //if((Str433_flag.master_sta==1)||(Str433_flag.remote_sta==1))//��������ƥ��ɹ���ң����ƥ�����н��յ����壻
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
   IWDG->KR=0xAA;//ι��
    
 }   

/**���ƴ���**/
 if(StrKey_flag.key_countFlag==0 && (Str433_flag.data_sta==nowork )){  //����������ڽ��б��ֿ��Ʋ����ı�ֱ����������
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


/******************* (C) COPYRIGHT EW������ *****END OF FILE******************/
