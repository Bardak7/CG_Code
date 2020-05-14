#include "sys.h"

u8 Tmp_Rx_Buf[200];   //接收数据处理区
WIFI_DATA Wifi_SendData;
u8 SendIndex;
u8 SendIndexAll;

u8 Rx_Buf[200];       //接收缓冲
u8 Rx_Index;   
u8 rev_cnt;           //WIFI 4ms超时连接计数
u16 hlw_erst_dly;
enum rev_func Is_rev_state; 
u8 StudyCnt;
extern IrOrRf_DATA ir_txdata;
IrStudy_DATA IrStudySendData;
extern SysStruct SysSetData;
extern IrOrRf_DATA rf433_txdata;
//======================================================
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 

//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0)//;//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	  //上拉
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
	//=================GPIO端口设置=================
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

	//=================Usart1 NVIC 配置==================
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级3级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	                          //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器USART1
  
  //USART 初始化设置 
	USART_InitStructure.USART_BaudRate = bound;               //一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//8位数据
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//1位停止
	USART_InitStructure.USART_Parity = USART_Parity_No;			  //无校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;    //硬件流控制-none
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //接受及发送使能

  USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);            //使能接收中断
//USART_ITConfig(USART1, USART_IT_TXE, ENABLE);			        //使能发送缓冲空中断
//USART_ITConfig(USART1, USART_IT_TC, ENABLE);				      //使能发送完成中断
	USART_Cmd(USART1, ENABLE);                                //使能串口
}

void Send_wifi(void)
{
	u8 *p;
	if((SysSetData.McuToApp_flag)&&(SysSetData.Rf315Send_flag==0)&&(SysSetData.Rf433Send_flag==0)&&(SysSetData.IrStudySendFlag==0)\
		&&(SysSetData.IrSend_flag==0)||SendIndex)
	{ 
		p = (u8 *)(&Wifi_SendData);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); //发送完成
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
  WSVers = 0x0001; //版本1
  WSCmd = WRCmd;
	WSSeril = WRSeril;
	WSRes0 = 0x00000000;
	WSRes1 = 0x00000000;
}

//====  串口数据处理函数  ===// 
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
      ((Tmp_Rx_Buf[11]+(Tmp_Rx_Buf[10]<<8))==(Rx_Index-21)))//确认头和长度一致
    { 
      switch(Tmp_Rx_Buf[7]+(Tmp_Rx_Buf[6]<<8))
      { //云端与主机通讯命令
//         case 0x0000: //负载获取ip
//              break;
//         case 0x0001: //主机登录   
//              break;
// 				case 0x0002: //心跳上报
//              break;
//         case 0x0003: //固件升级  
//              break;
// 				case 0x0004: //UDP穿透握手  
//              break;
				//主机与客户端通讯命令
				case 0x2001: //广播命令(定时)
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
        case 0x2002: //添加分机  
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
 				case 0x2003: //学习命令
					   StudyCnt=10;
             break;
//        case 0x2004: //停止学习 
//             break;
				case 0x2005: //控制设备,现特指发送已学习指令
					   //从存储区将要发送的学习码，赋值发送区
				     STMFLASH_Read(IRSTUDY_SAVE_ADDR+Tmp_Rx_Buf[25]*IRSTUDY_SAVE_UNIT,(u16 *)&IrStudySendData,28);					 
				     SysSetData.IrStudySendFlag=1;
				
				     //int
				     Wifi_SendData.Data[0] = 0x00;
				     Wifi_SendData.Data[1] = 0x00;
					   //Addr
				     Wifi_SendData.Data[2] = 0x00;//状态
				     //CRC
				     Wifi_SendData.Data[3] = 0x71;
	           Wifi_SendData.Data[4] = 0x21;
				
				     WSLen = 6;
	           SendIndex=0;
				     SendIndexAll=22+WSLen;
				     WifiTxSocket();
				     SysSetData.McuToApp_flag=1;
             break;
				case 0x2006: //锁定/解锁设备
					   if(SysSetData.bLock==0)
					     SysSetData.bLock=1;
						 else
							 SysSetData.bLock=0;
						 
						 //int
				     Wifi_SendData.Data[0] = 0x00;
				     Wifi_SendData.Data[1] = 0x00;
					   //Addr
				     Wifi_SendData.Data[2] = 0x00;//状态
				     //CRC
				     Wifi_SendData.Data[3] = 0x71;
	           Wifi_SendData.Data[4] = 0x21;
				
				     WSLen = 6;
	           SendIndex=0;
				     SendIndexAll=22+WSLen;
				     WifiTxSocket();
				     SysSetData.McuToApp_flag=1;
             break;
//         case 0x2007: //固件升级  
// 					
//              break;
				case 0x2008: //查询所有主机/分机状态
					   SysSetData.AllAskMorSFlag=1;
					   rf433_txdata.DistAddr=((SysSetData.SysAddr&0x000000ff)<<8)+SysSetData.SubNum+0xff;
				     rf433_txdata.Type=0x01;//fenji
				     rf433_txdata.Cmd=0x22;//查询
				     rf433_txdata.Len=0x00;
				     p=(u8 *)&rf433_txdata;
				     for(i=0;i<rf433_txdata.Len+7;i++)//crc
				       rf433_txdata.Data[0]+=*(p+i);
				     JudgeRf433Busy();
             break;
        case 0x2009: //批量查询主机/分机状态  
					   
				
             break;
				case 0x200a: //添加定时任务
					
             break;
				case 0x200b: //删除定时任务 
					
             break;
				case 0x200c: //修改定时任务
					
             break;
        case 0x200d: //获取定时任务列表
					
             break;
				case 0x200e: //获取单个定时任务
					
             break;
				case 0x200f: //修改定时任务状态
					
             break;
// 				case 0x2010: //添加开关
//              break;
        case 0x2011: //批量查询开关状态
             break;
// 				case 0x2012: //控制开关
//              break;
// 				case 0x2013: //获取升级进度
// 					
//              break;
        default:break;
      } 
    }
    Rx_Index=0;
  }
}  

//============串口1中断服务程序============
void USART1_IRQHandler(void)                	
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)   //接收中断
	{	
		Rx_Buf[Rx_Index++]=USART_ReceiveData(USART1); //读取数据
		Is_rev_state = inwork;
		rev_cnt = 0;
// 		hlw_erst_dly=0;//
  
		if(Rx_Index>200) 
		  Rx_Index=0; 		 
		USART_ClearITPendingBit(USART1, USART_IT_RXNE); 
  } 

	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)   //这段是为了避免STM32 USART 第一个字节发不出去的BUG 
	{ 
	  USART_ITConfig(USART1, USART_IT_TXE, DISABLE);		//禁止发缓冲器空中断
		//USART_ClearITPendingBit(USART1, USART_IT_TXE);
	}
} 



