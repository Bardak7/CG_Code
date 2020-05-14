#include "sys.h"

u8 Tmp_Rx_Buf[200];   //�������ݴ�����
WIFI_DATA Wifi_SendData;
u8 SendIndex;
u8 SendIndexAll;

u8 Rx_Buf[200];       //���ջ���
u8 Rx_Index;   
u8 rev_cnt;           //WIFI 4ms��ʱ���Ӽ���
u16 hlw_erst_dly;
enum rev_func Is_rev_state; 
u8 StudyCnt;
extern IrOrRf_DATA ir_txdata;
IrStudy_DATA IrStudySendData;
extern SysStruct SysSetData;
extern IrOrRf_DATA rf433_txdata;
//======================================================
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 

//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0)//;//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
int fputc(int ch, FILE *f)
{
	Wifi_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}

void HlwIo_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //gp1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	  //����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //stat
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_11);//exdef
}

void U1_Init(u32 bound)
{
	//=================GPIO�˿�����=================
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);
	
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

	//=================Usart1 NVIC ����==================
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	                          //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���USART1
  
  //USART ��ʼ������ 
	USART_InitStructure.USART_BaudRate = bound;               //һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//8λ����
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//1λֹͣ
	USART_InitStructure.USART_Parity = USART_Parity_No;			  //��У��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;    //Ӳ��������-none
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //���ܼ�����ʹ��

  USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);            //ʹ�ܽ����ж�
//USART_ITConfig(USART1, USART_IT_TXE, ENABLE);			        //ʹ�ܷ��ͻ�����ж�
//USART_ITConfig(USART1, USART_IT_TC, ENABLE);				      //ʹ�ܷ�������ж�
	USART_Cmd(USART1, ENABLE);                                //ʹ�ܴ���
}

void Send_wifi(void)
{
	u8 *p;
	if((SysSetData.McuToApp_flag)&&(SysSetData.Rf315Send_flag==0)&&(SysSetData.Rf433Send_flag==0)&&(SysSetData.IrStudySendFlag==0)\
		&&(SysSetData.IrSend_flag==0)||SendIndex)
	{ 
		p = (u8 *)(&Wifi_SendData);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); //�������
		USART_SendData(USART1,*(p+(SendIndex++)));
      
		if(SendIndex==SendIndexAll)
		{		
		  SysSetData.McuToApp_flag=0;
			if(SysSetData.WifiToRf433Flag)
			{
				SysSetData.WifiToRf433Flag=0;
				JudgeRf433Busy();
				//SysSetData.Rf433Send_flag=1;
			}
		}
  }//if(McuToApp_flag==TRUE)
}  

void WifiTxSocket(void)
{
	WSHead = 0x24676340;
  WSVers = 0x0001; //�汾1
  WSCmd = WRCmd;
	WSSeril = WRSeril;
	WSRes0 = 0x00000000;
	WSRes1 = 0x00000000;
}

//====  �������ݴ�����  ===// 
void Task_wifi(void)
{
  u8 i,*p;
    
  if(Is_rev_state==workok)
  {
    Is_rev_state=nowork;
    
    for(i=0;i<Rx_Index;)
    {
      Tmp_Rx_Buf[i]=0;
      Tmp_Rx_Buf[i]=Rx_Buf[i];
      Rx_Buf[i]=0;
      i++;
    }
    
    if(((Tmp_Rx_Buf[0])==0x24)&&((Tmp_Rx_Buf[1])==0x67)&&\
      ((Tmp_Rx_Buf[2])==0x63)&&((Tmp_Rx_Buf[3])==0x40)&&\
      ((Tmp_Rx_Buf[11]+(Tmp_Rx_Buf[10]<<8))==(Rx_Index-21)))//ȷ��ͷ�ͳ���һ��
    { 
      switch(Tmp_Rx_Buf[7]+(Tmp_Rx_Buf[6]<<8))
      { //�ƶ�������ͨѶ����
//         case 0x0000: //���ػ�ȡip
//              break;
//         case 0x0001: //������¼   
//              break;
// 				case 0x0002: //�����ϱ�
//              break;
//         case 0x0003: //�̼�����  
//              break;
// 				case 0x0004: //UDP��͸����  
//              break;
				//������ͻ���ͨѶ����
				case 0x2001: //�㲥����(��ʱ)
					   //int
				     Wifi_SendData.Data[0] = 0x00;
				     Wifi_SendData.Data[1] = 0x00;
					   //Addr
					   Wifi_SendData.Data[2] = (u8)((SysSetData.SysAddr&0xff000000)>>24);
				     Wifi_SendData.Data[3] = (u8)((SysSetData.SysAddr&0x00ff0000)>>16);
				     Wifi_SendData.Data[4] = (u8)((SysSetData.SysAddr&0x0000ff00)>>8);
				     Wifi_SendData.Data[5] = (u8)SysSetData.SysAddr&0x000000ff;
				     //CRC
				     Wifi_SendData.Data[6] = 0x71;
	           Wifi_SendData.Data[7] = 0x21;
				
				     WSLen = 6;
	           SendIndex=0;
				     SendIndexAll=22+WSLen;
				     WifiTxSocket();
				     SysSetData.McuToApp_flag=1;
             break;
        case 0x2002: //��ӷֻ�  
				     rf433_txdata.DistAddr=((SysSetData.SysAddr&0x000000ff)<<8)+SysSetData.SubNum+SysSetData.TerminalNum+1;
				     rf433_txdata.Type=0x01;//fenji
				     rf433_txdata.Cmd=0x21;
				     rf433_txdata.Len=0x00;
				     p=(u8 *)&rf433_txdata;
				     for(i=0;i<rf433_txdata.Len+7;i++)//crc
				       rf433_txdata.Data[0]+=*(p+i);
				     JudgeRf433Busy();
				     //SysSetData.Rf433Send_flag=1;
             break;
 				case 0x2003: //ѧϰ����
					   StudyCnt=10;
             break;
//        case 0x2004: //ֹͣѧϰ 
//             break;
				case 0x2005: //�����豸,����ָ������ѧϰָ��
					   //�Ӵ洢����Ҫ���͵�ѧϰ�룬��ֵ������
				     STMFLASH_Read(IRSTUDY_SAVE_ADDR+Tmp_Rx_Buf[25]*IRSTUDY_SAVE_UNIT,(u16 *)&IrStudySendData,28);					 
				     SysSetData.IrStudySendFlag=1;
				
				     //int
				     Wifi_SendData.Data[0] = 0x00;
				     Wifi_SendData.Data[1] = 0x00;
					   //Addr
				     Wifi_SendData.Data[2] = 0x00;//״̬
				     //CRC
				     Wifi_SendData.Data[3] = 0x71;
	           Wifi_SendData.Data[4] = 0x21;
				
				     WSLen = 6;
	           SendIndex=0;
				     SendIndexAll=22+WSLen;
				     WifiTxSocket();
				     SysSetData.McuToApp_flag=1;
             break;
				case 0x2006: //����/�����豸
					   if(SysSetData.bLock==0)
					     SysSetData.bLock=1;
						 else
							 SysSetData.bLock=0;
						 
						 //int
				     Wifi_SendData.Data[0] = 0x00;
				     Wifi_SendData.Data[1] = 0x00;
					   //Addr
				     Wifi_SendData.Data[2] = 0x00;//״̬
				     //CRC
				     Wifi_SendData.Data[3] = 0x71;
	           Wifi_SendData.Data[4] = 0x21;
				
				     WSLen = 6;
	           SendIndex=0;
				     SendIndexAll=22+WSLen;
				     WifiTxSocket();
				     SysSetData.McuToApp_flag=1;
             break;
//         case 0x2007: //�̼�����  
// 					
//              break;
				case 0x2008: //��ѯ��������/�ֻ�״̬
					   SysSetData.AllAskMorSFlag=1;
					   rf433_txdata.DistAddr=((SysSetData.SysAddr&0x000000ff)<<8)+SysSetData.SubNum+0xff;
				     rf433_txdata.Type=0x01;//fenji
				     rf433_txdata.Cmd=0x22;//��ѯ
				     rf433_txdata.Len=0x00;
				     p=(u8 *)&rf433_txdata;
				     for(i=0;i<rf433_txdata.Len+7;i++)//crc
				       rf433_txdata.Data[0]+=*(p+i);
				     JudgeRf433Busy();
             break;
        case 0x2009: //������ѯ����/�ֻ�״̬  
					   
				
             break;
				case 0x200a: //��Ӷ�ʱ����
					
             break;
				case 0x200b: //ɾ����ʱ���� 
					
             break;
				case 0x200c: //�޸Ķ�ʱ����
					
             break;
        case 0x200d: //��ȡ��ʱ�����б�
					
             break;
				case 0x200e: //��ȡ������ʱ����
					
             break;
				case 0x200f: //�޸Ķ�ʱ����״̬
					
             break;
// 				case 0x2010: //��ӿ���
//              break;
        case 0x2011: //������ѯ����״̬
             break;
// 				case 0x2012: //���ƿ���
//              break;
// 				case 0x2013: //��ȡ��������
// 					
//              break;
        default:break;
      } 
    }
    Rx_Index=0;
  }
}  

//============����1�жϷ������============
void USART1_IRQHandler(void)                	
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)   //�����ж�
	{	
		Rx_Buf[Rx_Index++]=USART_ReceiveData(USART1); //��ȡ����
		Is_rev_state = inwork;
		rev_cnt = 0;
// 		hlw_erst_dly=0;//
  
		if(Rx_Index>200) 
		  Rx_Index=0; 		 
		USART_ClearITPendingBit(USART1, USART_IT_RXNE); 
  } 

	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)   //�����Ϊ�˱���STM32 USART ��һ���ֽڷ�����ȥ��BUG 
	{ 
	  USART_ITConfig(USART1, USART_IT_TXE, DISABLE);		//��ֹ�����������ж�
		//USART_ClearITPendingBit(USART1, USART_IT_TXE);
	}
} 



