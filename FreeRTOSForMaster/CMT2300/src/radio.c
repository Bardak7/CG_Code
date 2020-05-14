/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, CMOSTEK SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * Copyright (C) CMOSTEK SZ.
 */

/*!
 * @file    radio.c
 * @brief   Generic radio handlers
 *
 * @version 1.2
 * @date    Jul 17 2017
 * @author  CMOSTEK R@D
 */
 
#include "radio.h"
#include "cmt2300a_params_fsk.h"
#include "cmt2300_defs.h"
#include "cmt2300_hal.h"
#include <string.h>
#include "sys.h"
#include "protocol.h"
#include "FreeRTOS.h"
//#include "portmacro.h"
//#include "IRorRF.h"
//static EnumRFStatus g_nNextRFState = RF_STATE_IDLE;
//static u8* g_pRxBuffer = NULL;
//static u8* g_pTxBuffer = NULL;
//static u16 g_nRxLength = 0;
//static u16 g_nTxLength = 0;

//static u32 g_nRxTimeout = INFINITE;
//static u32 g_nTxTimeout = INFINITE;
//static u32 g_nRxTimeCount = 0;
//static u32 g_nTxTimeCount = 0;

//static u8 g_nInterrutFlags = 0;

uint32_t TickCount;
//extern u8 rf433_data[300];
//extern u8 n_gCleanSyc; //Syn word归零标记
//extern SysStruct SysSetData;
void RF_Init_FSK(void)
{
    u8 tmp;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
//	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
//    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
		portENTER_CRITICAL();
		
		
    CMT2300A_InitGpio();

    
    /* Config registers */
    Cmt2300_ConfigRegBank(CMT2300_CMT_BANK_ADDR       , g_cmt2300aCmtBank       , CMT2300_CMT_BANK_SIZE       );
    Cmt2300_ConfigRegBank(CMT2300_SYSTEM_BANK_ADDR    , g_cmt2300aSystemBank    , CMT2300_SYSTEM_BANK_SIZE    );
    Cmt2300_ConfigRegBank(CMT2300_FREQUENCY_BANK_ADDR , g_cmt2300aFrequencyBank , CMT2300_FREQUENCY_BANK_SIZE );
    Cmt2300_ConfigRegBank(CMT2300_DATA_RATE_BANK_ADDR , g_cmt2300aDataRateBank  , CMT2300_DATA_RATE_BANK_SIZE );
    Cmt2300_ConfigRegBank(CMT2300_BASEBAND_BANK_ADDR  , g_cmt2300aBasebandBank  , CMT2300_BASEBAND_BANK_SIZE  );
    Cmt2300_ConfigRegBank(CMT2300_TX_BANK_ADDR        , g_cmt2300aTxBank        , CMT2300_TX_BANK_SIZE        );
    
    // xosc_aac_code[2:0] = 2
    tmp = (~0x07) & Cmt2300_ReadReg(CMT2300_CUS_CMT10);
    Cmt2300_WriteReg(CMT2300_CUS_CMT10, tmp|0x02);
		RF_Config();
		RF_Receive();
		portEXIT_CRITICAL();
}

void RF_Config(void)
{
#ifdef ENABLE_ANTENNA_SWITCH
    /* If you enable antenna switch, GPIO1/GPIO2 will output RX_ACTIVE/TX_ACTIVE,
       and it can't output INT1/INT2 via GPIO1/GPIO2 */
    Cmt2300_EnableAnte nnaSwitch(0);
    
#else
    /* Config GPIOs */
    Cmt2300_ConfigGpio(
        CMT2300_GPIO1_SEL_INT1 | /* INT1 > GPIO1 */
        CMT2300_GPIO2_SEL_INT2 | /* INT2 > GPIO2 */
        CMT2300_GPIO3_SEL_DOUT
        );
    
    /* Config interrupt */
    Cmt2300_ConfigInterrupt(
        CMT2300_INT_SEL_TX_DONE, /* Config INT1 */
        CMT2300_INT_SEL_CRC_OK//CMT2300_INT_SEL_PKT_OK   /* Config INT2 */
        );
#endif

    /* Enable interrupt */
    Cmt2300_EnableInterrupt(
        CMT2300_MASK_TX_DONE_EN  |
        CMT2300_MASK_PREAM_OK_EN |
        CMT2300_MASK_SYNC_OK_EN  |
        CMT2300_MASK_NODE_OK_EN  |
        CMT2300_MASK_CRC_OK_EN   |
        CMT2300_MASK_PKT_DONE_EN
        );
    
    /* Disable low frequency OSC calibration */
    Cmt2300_EnableLfosc(FALSE);
    
    /* Use a single 64-byte FIFO for either Tx or Rx */
    Cmt2300_EnableFifoMerge(TRUE);
    
    //CMT2300A_SetFifoThreshold(16);
    
    /* This is optional, only needed when using Rx fast frequency hopping */
    /* See AN142 and AN197 for details */
    //CMT2300A_SetAfcOvfTh(0x27);
    /*Set Synchronized word length*/
		Cmt2300_SetSyncLength(4);  //4个字节的sync长度
		Cmt2300_SetSynWord((u32)(st_gHostInfo.deviceID));
    /* Go to sleep for configuration to take effect */
    Cmt2300_GoSleep();
}
/*
void RF_SetStatus(EnumRFStatus nStatus)
{
    g_nNextRFState = nStatus;

}

EnumRFStatus RF_GetStatus(void)
{
    return g_nNextRFState;
}

u8 RF_GetInterruptFlags(void)
{
    return g_nInterrutFlags;
}

void RF_StartRx(u8 buf[], u16 len, u32 timeout)
{
	  ANT_RX();
    g_pRxBuffer = buf;
    g_nRxLength = len;
    g_nRxTimeout = timeout;
    
    memset(g_pRxBuffer, 0, g_nRxLength);
    
    g_nNextRFState = RF_STATE_RX_START;
}

void RF_StartTx(u8 buf[], u16 len, u32 timeout)
{
	  ANT_TX()
    g_pTxBuffer = buf;
    g_nTxLength = len;
    g_nTxTimeout = timeout;
    
    g_nNextRFState = RF_STATE_TX_START;
}
*/

void ProcessErro(void)
{
	Cmt2300_SoftReset();
	delay_xms(20);
	
	Cmt2300_GoStby();
	RF_Config();
}

void RF_Send(u8 buf[], u16 len)
{
	ANT_TX();
	portENTER_CRITICAL();
	Cmt2300_GoStby();
	Cmt2300_ClearInterruptFlags();
	
	/* Must clear FIFO after enable SPI to read or write the FIFO */
	Cmt2300_EnableWriteFifo();
	CMT2300A_ClearTxFifo();
	
	/* The length need be smaller than 32 */
	Cmt2300_WriteFifo(buf, len);
	if( 0==(CMT2300_MASK_TX_FIFO_NMTY_FLG & Cmt2300_ReadReg(CMT2300_CUS_FIFO_FLAG)) )
		//g_nNextRFState = RF_STATE_ERROR; //如果0x6E的TX_FIFO_NMTY_FLG＝＝0的话那么就失败了
		ProcessErro();
	if(FALSE==Cmt2300_GoTx())
//			g_nNextRFState = RF_STATE_ERROR; //如果
		ProcessErro(); //如果Cmt2300_GoTx错误就重启模块
//	else
//			g_nNextRFState = RF_STATE_TX_WAIT;
	
//	g_nTxTimeCount = CMT2300A_GetTickCount();
             
	portEXIT_CRITICAL();
}
void RF_Receive(void) 
{
	ANT_RX();
	portENTER_CRITICAL();
	Cmt2300_GoStby();
	Cmt2300_ClearInterruptFlags();
	
	/* Must clear FIFO after enable SPI to read or write the FIFO */
	Cmt2300_EnableReadFifo();
	CMT2300A_ClearRxFifo();
	
	if(FALSE==Cmt2300_GoRx())
		ProcessErro(); //重启模块
	portEXIT_CRITICAL();
}

u8 RF_Read(u8 buf[], u16 len)
{		
	u8 g_nInterrutFlags = 0;
		Cmt2300_GoStby();

		/* The length need be smaller than 32 */
		Cmt2300_ReadFifo(buf, len);

		g_nInterrutFlags = Cmt2300_ClearInterruptFlags();
		return 	g_nInterrutFlags;
}
//EnumRFResult RF_Process(void)
//{
//    EnumRFResult nRes = RF_BUSY;
////    u8 i;
//    switch(g_nNextRFState) 
//    {
//    case RF_STATE_IDLE:
//    {
//        nRes = RF_IDLE;
//        break;
//    }
//    
//    case RF_STATE_RX_START:
//    {
//        Cmt2300_GoStby();
//        Cmt2300_ClearInterruptFlags();
//        
//        /* Must clear FIFO after enable SPI to read or write the FIFO */
//        Cmt2300_EnableReadFifo();
//        CMT2300A_ClearRxFifo();
//        
//        if(FALSE==Cmt2300_GoRx())
//            g_nNextRFState = RF_STATE_ERROR;
//        else
//            g_nNextRFState = RF_STATE_RX_WAIT;
//        
//        g_nRxTimeCount = CMT2300A_GetTickCount();
//        
//        break;
//    }
//    
//    case RF_STATE_RX_WAIT:
//    {
//#ifdef ENABLE_ANTENNA_SWITCH
//        if(Cmt2300_MASK_PKT_OK_FLG & Cmt2300_ReadReg(Cmt2300_CUS_INT_FLAG))  /* Read PKT_OK flag */
//#else
//        if(CMT2300A_ReadGpio2())  /* Read INT2, PKT_OK */
//#endif
//        {
//            g_nNextRFState = RF_STATE_RX_DONE;
//        }
//        
//        if( (INFINITE != g_nRxTimeout) && ((TickCount-g_nRxTimeCount) > g_nRxTimeout) ) //超时返回TIMOUT	
//            g_nNextRFState = RF_STATE_RX_TIMEOUT;
//        
//        break;
//    }
//    
//    case RF_STATE_RX_DONE:
//    {
//        Cmt2300_GoStby();

//        /* The length need be smaller than 32 */
//        Cmt2300_ReadFifo(g_pRxBuffer, g_nRxLength);

//        g_nInterrutFlags = Cmt2300_ClearInterruptFlags();
//            
//        Cmt2300_GoSleep();
//        
//        g_nNextRFState = RF_STATE_IDLE;
//        nRes = RF_RX_DONE;
//        break;
//    }
//    
//    case RF_STATE_RX_TIMEOUT:
//    {
//        Cmt2300_GoSleep();
//        
//        g_nNextRFState = RF_STATE_IDLE;
//        nRes = RF_RX_TIMEOUT;
//        break;
//    }
//    
//    case RF_STATE_TX_START:
//    {
//        Cmt2300_GoStby();
//        Cmt2300_ClearInterruptFlags();
//        
//        /* Must clear FIFO after enable SPI to read or write the FIFO */
//        Cmt2300_EnableWriteFifo();
//        CMT2300A_ClearTxFifo();
//        
//        /* The length need be smaller than 32 */
//        Cmt2300_WriteFifo(g_pTxBuffer, g_nTxLength);
//        
////			  Cmt2300_EnableTxFifoRead(TRUE);//TXfifo可读
////			  Cmt2300_ReadFifo(rf433_data, g_nTxLength);
//////			
////			  for(i=0;i<g_nTxLength;i++){
////				  if(rf433_data[i]!=g_pTxBuffer[i]){
////					  break;
////					}
////				}
////				Cmt2300_EnableTxFifoRead(FALSE);//退出TXfifo可读
////				Cmt2300_EnableWriteFifo();
////        CMT2300A_ClearTxFifo();
////				Cmt2300_WriteFifo(g_pTxBuffer, g_nTxLength);
//        if( 0==(CMT2300_MASK_TX_FIFO_NMTY_FLG & Cmt2300_ReadReg(CMT2300_CUS_FIFO_FLAG)) )
//            g_nNextRFState = RF_STATE_ERROR; //如果0x6E的TX_FIFO_NMTY_FLG＝＝0的话那么就失败了

//        if(FALSE==Cmt2300_GoTx())
//            g_nNextRFState = RF_STATE_ERROR; //如果
//        else
//            g_nNextRFState = RF_STATE_TX_WAIT;
//        
//        g_nTxTimeCount = CMT2300A_GetTickCount();
//        
//        break;
//    }
//        
//    case RF_STATE_TX_WAIT:
//    {
//#ifdef ENABLE_ANTENNA_SWITCH
//        if(Cmt2300_MASK_TX_DONE_FLG & Cmt2300_ReadReg(Cmt2300_CUS_INT_CLR1))  /* Read TX_DONE flag */
//#else
//        if(CMT2300A_ReadGpio1())  /* Read INT1, TX_DONE */
//#endif
//        {
//            g_nNextRFState = RF_STATE_TX_DONE;
//        }
//        
//        if( (INFINITE != g_nTxTimeout) && ((TickCount-g_nTxTimeCount) > g_nTxTimeout) )
//            g_nNextRFState = RF_STATE_TX_TIMEOUT;
//            
//        break;
//    }
//            
//    case RF_STATE_TX_DONE:
//    {
//        Cmt2300_ClearInterruptFlags();
////        Cmt2300_GoSleep();
//        if(n_gCleanSyc==1){//如果发送完成需要对synword清零
//				  n_gCleanSyc=0;
//          Cmt2300_GoStby();
//					Cmt2300_SetSynWord((u32)(st_gHostInfo.deviceID));
//					Cmt2300_GoSleep();					
//				}
//        g_nNextRFState = RF_STATE_IDLE;
//        nRes = RF_TX_DONE;
//        break;
//    }
//    
//    case RF_STATE_TX_TIMEOUT:
//    {
//        Cmt2300_GoSleep();
//        
//        g_nNextRFState = RF_STATE_IDLE;
//        nRes = RF_TX_TIMEOUT;
//        break;
//    }
//    
//    case RF_STATE_ERROR:
//    {
//        Cmt2300_SoftReset();
//        delay_ms(20);
//        
//        Cmt2300_GoStby();
//        RF_Config();
//        
//        g_nNextRFState = RF_STATE_IDLE;
//        nRes = RF_ERROR;
//        break;
//    }
//    
//    default:
//        break;
//    }
//    
//    return nRes;
//}  //*/

