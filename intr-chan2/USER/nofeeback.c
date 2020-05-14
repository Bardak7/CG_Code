

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "sysclock.h"
#include "stm8s_exti.h"
#include "uart.h"
#include "stm8s_gpio.h"
#include "delay.h"
#include "string.h"
#include "stdlib.h"
#include "trans433.h"
/* Private defines -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**Touch����**/
#define T_MODE(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_5):GPIO_WriteLow(GPIOC,GPIO_PIN_5)//�궨�崥��ģʽ:������ͷ�����
#define T_SLPEN(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_5):GPIO_WriteLow(GPIOC,GPIO_PIN_5) //��˯��
#define Touch1 (GPIOA->IDR&0x02)
#define Touch2 (GPIOA->IDR&0x04)
#define Touch3 (GPIOA->IDR&0x08)

/**LED�ĺ궨��**/
#define LED3(n) n&1?GPIO_WriteLow(GPIOB,GPIO_PIN_5):GPIO_WriteHigh(GPIOB,GPIO_PIN_5)
#define LED2(n) n&1?GPIO_WriteLow(GPIOB,GPIO_PIN_4):GPIO_WriteHigh(GPIOB,GPIO_PIN_4)
#define LED1(n) n&1?GPIO_WriteLow(GPIOD,GPIO_PIN_3):GPIO_WriteHigh(GPIOD,GPIO_PIN_3)
#define LED0(n) n&1?GPIO_WriteLow(GPIOA,GPIO_PIN_3):GPIO_WriteHigh(GPIOA,GPIO_PIN_3)
/**����ĺ궨��**/
#define OUT1(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_6):GPIO_WriteLow(GPIOC,GPIO_PIN_6)
#define OUT2(n) n&1?GPIO_WriteHigh(GPIOC,GPIO_PIN_7):GPIO_WriteLow(GPIOC,GPIO_PIN_7)
#define OUT3(n) n&1?GPIO_WriteHigh(GPIOD,GPIO_PIN_1):GPIO_WriteLow(GPIOD,GPIO_PIN_1)

/**һ֡����7���ֽ�**/
#define S_ADDR  Trans_433.SendBuff[0] //Դ��ַ
#define T_ADDR  Trans_433.SendBuff[4] //target��ַ
#define D_TYPE  Trans_433.SendBuff[8] //�豸���� 0x00������0x01�м̣�0x02����
#define C_TYPE  Trans_433.SendBuff[9] //�������� 0x11����0x12�أ�0x21��ӷֻ���0x22��ӿ���
#define D_LEN   Trans_433.SendBuff[10] //���ݳ���
#define D_DATA1 Trans_433.SendBuff[11] //������  ��λ��ʾ�������ص�״̬
#define D_DATA2 Trans_433.SendBuff[12] 
#define D_CHECK Trans_433.SendBuff[13] //����У��

/**Device Type**/
#define Master 0x00  //����
#define Rep    0x01  //�м�
#define Switch 0x02  //����
/**Cmd Type**/
#define  SW_ON     0x91 //���ؿ����λΪ1
#define  SW_OFF    0x92 //���ع����λΪ1
#define  ADD_SW    0x21 //��ӷֻ�

Str_433 Trans_433=
     { /* Trans_433.Read_Buff[14]=*/{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
      /*  Trans_433.Send_Buff[14]=*/{0x01,0x01,0x01,0x01,  0x10,0x10,0x10,0x10,     0x02,     0x11,       0x01,    0x02,0x5A,   0x00},
        0
     };
  Str_433* Point_433;

/**�������������Ƿ�һ��**/
void Check_addr(u8* data)
{
  u8 i;
  for(i=4;i<8;i++)
  {
    if(data[i]==Trans_433.Send_Buff1[i]) data[i]|=0x10;
    else Trans_433.STA&=(~0x10);
  }
}//*/

void GPIO_init(void)
{
  GPIO_Init(GPIOA,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_FAST);//�������ϵ�LED��
  
  /**LED���ų�ʼ��**/
  GPIO_Init(GPIOB,GPIO_PIN_4|GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_FAST); //B4��LED2   B5:LED3
  GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_FAST);            //D3: LED1
  
  /**������ⲿ�����ų�ʼ��**/
  GPIO_Init(GPIOC,GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(GPIOD,GPIO_PIN_1,GPIO_MODE_OUT_PP_LOW_FAST); 
  
  /**�ⲿIC�������ų�ʼ��**/
  GPIO_Init(GPIOC,GPIO_PIN_4|GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_FAST); //C4:SLPEN C5:T_MODE
 
  /**433������ų�ʼ��**/
  GPIO_Init(GPIOC,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_FAST);  //GPIOC3���ж��������433������
  GPIO_Init(GPIOD,GPIO_PIN_2,GPIO_MODE_IN_FL_NO_IT);  //D2�����ⲿ�����433�ź�
  
  /**Touch���ų�ʼ��**/
  GPIO_Init(GPIOA,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_MODE_IN_PU_NO_IT);  //�Ӵ���оƬ�����
 
}
int main(void)
{
     u8 read;
     int i;
     
     /*                32λԴ��ַ��        32λĿ�꣬         �豸���ͣ��������ͣ����ݳ��ȣ������壬   ������*/
     //u8 data_array[]={0x01,0x01,0x01,0x01,  0x10,0x10,0x10,0x10,     0x02,     0x21,       0x02,    0x03,0x01,   0x7B};
     //u8 data_array2[]={'Y', 'U','A', 'N',  'J', 'i','a','n' ,'S', 'h','u','H','H'};
     u8 data_array1[]={0x01, 0x01,0x01,0x01,0x10 ,0x10 ,0x10 ,0x10 ,0x02 ,0x22 ,0x02 ,0x03 ,0x6D};
     u8 Touch=0;
     u8 Check=0;
     Str_433 * Point_433=&Trans_433;
	//TIM_CNT = 0;
	/* ϵͳʱ�ӳ�ʼ�� */
	SystemClock_Init();
      
	delay_init(16);  //��ʱ�����ĳ�ʼ��
        GPIO_init();   //��ʼ����Ӧ��GPIO��
	//TIM1_Init();
	Uart1_Init();         //��ʼ��UART1
	//disableInterrupts();
        //UART1_ITConfig(UART1_IT_RXNE_OR,ENABLE);
        UART1_SendString("System Init Finish!\n"); //���з�������
        //CLK_CCOConfig(CLK_OUTPUT_CPU);
        T_MODE(0);
        //T_SLPEN(1);       //enableInterrupts();   //���ж�
        //Trans_433.Send_Buff[8]=Switch; //D_TYPE �ǿ��ء�
        //Trans_433.Send_Buff[10]=0x01;     //D_LEN���ݳ���Ϊ1

    while(1)
    {
      Check=0;
      TX_433(Point_433->Send_Buff1,14);
     //TX_433(Trans_433,14);
       /**������������**
          check=0;
           if(Read_433()==0xFE)
           { UART1_SendString(temp);
             UART1_SendString("\r\n");/*
          if(Check_addr(data_array))//���Ŀ���ַ�ͱ���ַ��ͬ�������
             {
               for(i=0;i<12;i++)
                 check+=temp[i];
               if(temp[12]==check)
               {
                 Touch=temp[11];
                 /*if(temp[9]==0x11) //temp[3]�е�11��ʾ����12��ʾ��
                 {
                   Touch|=temp[12];
                 }
                 else if(temp[9]==0x12)
                 {
                   Touch&=(~temp[5]);
                 }*
                 TX_433(data_array1,13);
                 TX_433(data_array1,13);
                 //TX_433(data_array1,13);
               }
               else
                //UART1_SendString("У�����\r\n");
                 UART1_SendByte(0XFF);
             }
             else
               //UART1_SendString("�Ǳ���ַ������\r\n");
               UART1_SendByte(0xFE);
           
           } //*/
      if(!(Touch1))
      {
        if(Touch&0x01)
        {
          Touch&=~(0x01);   //�����һ������ǿ��ŵľͰ�������ˣ�����ǹ��ŵľͰ�������ˡ�
          Trans_433.Send_Buff1[9]=SW_OFF;
        }
        else
        {
          Touch|=0x01;
          Trans_433.Send_Buff1[9]=SW_ON;   //C_TYPE �ǿ������߹ء�
        }
        Trans_433.Send_Buff1[11]=Touch;  //���صĸ�·״̬
        for(i=0;i<12;i++)
        {
          Trans_433.Send_Buff1[12]+=Trans_433.Send_Buff1[i];
        }
        TX_433(Trans_433.Send_Buff1,14);
        //TX_433(Trans_433,14);
      }
      if(!(Touch2))
      {
        if(Touch&0x02)
        {
          Touch&=~(0x02);
          Trans_433.Send_Buff1[9]=SW_OFF;
        }
        else        
        {
          Touch|=0x02;
          Trans_433.Send_Buff2[9]=SW_ON;
        }
        Trans_433.Send_Buff2[11]=Touch;  //���صĸ�·״̬
        for(i=0;i<12;i++)
        {
          Trans_433.Send_Buff2[12]+=Trans_433.Send_Buff2[i];
        }
        TX_433(Point_433->Send_Buff2,14);
        //TX_433(Trans_433,14);
       } 
        if(!(Touch3))  //�����˵���������
        {
          if(Touch&0x04)
          {
            Touch&=~(0x04);
            Trans_433.Send_Buff3[9]=SW_OFF;
          }
          else
          {
            Touch|=0x04;
            Trans_433.Send_Buff3[9]=SW_ON;
          }
          Trans_433.Send_Buff3[11]=Touch;  //���صĸ�·״̬
        for(i=0;i<12;i++)
        {
          Trans_433.Send_Buff3[12]+=Trans_433.Send_Buff3[i];
        }
        TX_433(Point_433->Send_Buff3,14);
        //TX_433(Trans_433,14);
        i=0;
        /*
       while(1)
        {
          Point_433->STA=0x00;
          Read_433(Point_433);
          Check_addr(Point_433);
          if(Point_433->STA==0x1E)  //�ɹ��������� �˳�ѭ��
          {
            Point_433->STA=0x00;
            break;
          }
          else
          {
            delay_us(10);
            i++;
          }
          if(i>4000)        //���û��������50MS���ٷ�һ�Ρ�
          {
            i=0;
            TX_433(Trans_433,14);
           TX_433(Trans_433,14);
          }
        }//*/
        }
      Read_433(Point_433);//433��ȡ
      /**/
      Check_addr(Point_433->Read_Buff);
      if(Point_433->STA==0x1E)//������յ����ҵ�ַУ����ȷ
      {
        Point_433->STA=0x00;
        UART1_SendString(Trans_433.Read_Buff);
        UART1_SendString("\r\n");
        for(i=0;i<12;i++)
        {
          Check+=Point_433->Read_Buff[i];
        }
        if(Point_433->Read_Buff[12]==Check && (Point_433->Read_Buff[9]&0x90)) //�������ɹ���
        {
          Touch=Point_433->Read_Buff[11];  
          Point_433->Send_Buff1[12]=0x5A;
          
          //TX_433(Trans_433,14);
          //delay_ms(50);
          //TX_433(Trans_433,14);
          //UART1_SendString(Trans_433.Send_Buff);
          //UART1_SendString("\r\n");
        }
        
      }//*
      
      switch(Touch)
          {
            case 0x00: LED1(0);LED2(0);LED3(0);OUT1(0);OUT2(0);OUT3(0); break;
            case 0x01: LED1(1);LED2(0);LED3(0);OUT1(1);OUT2(0);OUT3(0); break;
            case 0x02: LED1(0);LED2(1);LED3(0);OUT1(0);OUT2(1);OUT3(0); break;
            case 0x03: LED1(1);LED2(1);LED3(0);OUT1(1);OUT2(1);OUT3(0); break;
            case 0x04: LED1(0);LED2(0);LED3(1);OUT1(0);OUT2(0);OUT3(1); break;
            case 0x05: LED1(1);LED2(0);LED3(1);OUT1(1);OUT2(0);OUT3(1); break;
            case 0x06: LED1(0);LED2(1);LED3(1);OUT1(0);OUT2(1);OUT3(1); break;
            case 0x07: LED1(1);LED2(1);LED3(1);OUT1(1);OUT2(1);OUT3(1); break;
          } //*/
      //delay_ms(100);
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
