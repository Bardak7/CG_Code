#include "cmt2300.h"
#include "cmt2300_defs.h"
#include "spi.h"
#include "cmt2300_hal.h"
//#include "exti.h"

#include "cmt2300a_params.h"

/*! ********************************************************
* @name    Cmt2300_SoftReset
* @desc    Soft reset.
* *********************************************************/
void Cmt2300_SoftReset(void)
{
    Cmt2300_WriteReg(0x7F, 0xFF);
}

/*! ********************************************************
* @name    Cmt2300_GetChipStatus
* @desc    Get the chip status.
* @return
*          CMT2300_STA_PUP
*          CMT2300_STA_SLEEP
*          CMT2300_STA_STBY
*          CMT2300_STA_RFS
*          CMT2300_STA_TFS
*          CMT2300_STA_RX
*          CMT2300_STA_TX
*          CMT2300_STA_EEPROM
*          CMT2300_STA_CAL
* *********************************************************/
u8 Cmt2300_GetChipStatus(void)
{
    return  Cmt2300_ReadReg(CMT2300_CUS_MODE_STA) & CMT2300_MASK_CHIP_MODE_STA;
}

/*! ********************************************************
* @name    Cmt2300_GetChipStatus
* @desc    Wait the chip status, and 50*200 us as timeout.
* @param   nStatus: the chip status
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_WaitChipStatus(u8 nStatus)
{
    u8 i;

    for(i=0; i<50; i++) 
	  {
        if(nStatus==Cmt2300_GetChipStatus())
            return TRUE;
		    delay_us(200);
    }

    return FALSE;
}

/*! ********************************************************
* @name    Cmt2300_GoSleep
* @desc    Entry SLEEP mode.
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_GoSleep(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_MODE_CTL, CMT2300_GO_SLEEP);
    return Cmt2300_WaitChipStatus(CMT2300_STA_SLEEP);
}

/*! ********************************************************
* @name    Cmt2300_GoStby
* @desc    Entry Sleep mode.
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_GoStby(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_MODE_CTL, CMT2300_GO_STBY);
    return Cmt2300_WaitChipStatus(CMT2300_STA_STBY);
}

/*! ********************************************************
* @name    Cmt2300_GoTFS
* @desc    Entry TFS mode.
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_GoTFS(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_MODE_CTL, CMT2300_GO_TFS);
    return Cmt2300_WaitChipStatus(CMT2300_STA_TFS);
}

/*! ********************************************************
* @name    Cmt2300_GoRFS
* @desc    Entry RFS mode.
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_GoRFS(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_MODE_CTL, CMT2300_GO_RFS);
    return Cmt2300_WaitChipStatus(CMT2300_STA_RFS);
}

/*! ********************************************************
* @name    Cmt2300_GoTx
* @desc    Entry Tx mode.
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_GoTx(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_MODE_CTL, CMT2300_GO_TX);
    return Cmt2300_WaitChipStatus(CMT2300_STA_TX);
}

/*! ********************************************************
* @name    Cmt2300_GoRx
* @desc    Entry Rx mode.
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_GoRx(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_MODE_CTL, CMT2300_GO_RX);
    return Cmt2300_WaitChipStatus(CMT2300_STA_RX);
}


/*! ********************************************************
* @name    Cmt2300_GoSwitch
* @desc    Change to Rx or Tx mode.
* @return  TRUE or FALSE
* *********************************************************/
BOOL Cmt2300_GoSwitch(void) //转换收发模式，并等候其转换后的状态
{   
	  u8 tmp;
	
	  if(CMT2300_STA_TX==Cmt2300_ReadReg(CMT2300_CUS_MODE_STA))//查询芯片状态
		{
		  tmp=CMT2300_STA_RX;
		}
		else 
			tmp=CMT2300_STA_TX;
    Cmt2300_WriteReg(CMT2300_CUS_MODE_CTL, CMT2300_GO_SWITCH);
	  
    return Cmt2300_WaitChipStatus(tmp);
}

BOOL CMT2300_ForceTX(void)
{
	u8 tmp;
	tmp=Cmt2300_ReadReg(CMT2300_CUS_MODE_STA)&0x0F;//查询现在状态，发送还是接收
	
	cmt_rx_ant_1();
	cmt_tx_ant_0();//天线引脚切换
	
  if(CMT2300_STA_TX==tmp)
	{
	   return TRUE;//如果已经是TX模式了不做更改
	}
	else if(CMT2300_STA_RX==tmp)
	{
	  return Cmt2300_GoSwitch();//转换收发模式，并等候其转换后的状态
	}
	else 
	{
		return Cmt2300_GoTx();
	}
}

BOOL CMT2300_ForceRX(void)
{
	u8 tmp;
	tmp=Cmt2300_ReadReg(CMT2300_CUS_MODE_STA)&0x0F;//查询现在状态，发送还是接收
	
	cmt_rx_ant_0();
	cmt_tx_ant_1();//天线引脚切换
	
  if(CMT2300_STA_RX==tmp)//如果已经是RX模式了不做更改
	{
	  return TRUE;
	}
	else if(CMT2300_STA_TX==tmp)
	{
	   return Cmt2300_GoSwitch();//转换收发模式，并等候其转换后的状态
	}
	else 
	{
		return Cmt2300_GoRx();
	}
}
/*! ********************************************************
* @name    Cmt2300_ConfigGpio
* @desc    Config GPIO pins mode.
* @param   nGpioSel: GPIO1_SEL | GPIO2_SEL | GPIO3_SEL | GPIO4_SEL
*          GPIO1_SEL:
*            CMT2300_GPIO1_SEL_DOUT/DIN 
*            CMT2300_GPIO1_SEL_INT1
*            CMT2300_GPIO1_SEL_INT2 
*            CMT2300_GPIO1_SEL_DCLK
*
*          GPIO2_SEL:
*            CMT2300_GPIO2_SEL_INT1 
*            CMT2300_GPIO2_SEL_INT2
*            CMT2300_GPIO2_SEL_DOUT/DIN 
*            CMT2300_GPIO2_SEL_DCLK
*
*          GPIO3_SEL:
*            CMT2300_GPIO3_SEL_CLKO 
*            CMT2300_GPIO3_SEL_DOUT/DIN
*            CMT2300_GPIO3_SEL_INT2 
*            CMT2300_GPIO3_SEL_DCLK
*
*          GPIO4_SEL:
*            CMT2300_GPIO4_SEL_RSTIN 
*            CMT2300_GPIO4_SEL_INT1
*            CMT2300_GPIO4_SEL_DOUT 
*            CMT2300_GPIO4_SEL_DCLK
* *********************************************************/
void Cmt2300_ConfigGpio(u8 nGpioSel)
{
    Cmt2300_WriteReg(CMT2300_CUS_IO_SEL, nGpioSel);
}

/*! ********************************************************
* @name    Cmt2300_ConfigInterrupt
* @desc    Config interrupt on INT1 and INT2.
* @param   nInt1Sel, nInt2Sel
*            CMT2300_INT_SEL_RX_ACTIVE
*            CMT2300_INT_SEL_TX_ACTIVE
*            CMT2300_INT_SEL_RSSI_VLD
*            CMT2300_INT_SEL_PREAM_OK
*            CMT2300_INT_SEL_SYNC_OK
*            CMT2300_INT_SEL_NODE_OK
*            CMT2300_INT_SEL_CRC_OK
*            CMT2300_INT_SEL_PKT_OK
*            CMT2300_INT_SEL_SL_TMO
*            CMT2300_INT_SEL_RX_TMO
*            CMT2300_INT_SEL_TX_DONE
*            CMT2300_INT_SEL_RX_FIFO_NMTY
*            CMT2300_INT_SEL_RX_FIFO_TH
*            CMT2300_INT_SEL_RX_FIFO_FULL
*            CMT2300_INT_SEL_RX_FIFO_WBYTE
*            CMT2300_INT_SEL_RX_FIFO_OVF
*            CMT2300_INT_SEL_TX_FIFO_NMTY
*            CMT2300_INT_SEL_TX_FIFO_TH
*            CMT2300_INT_SEL_TX_FIFO_FULL
*            CMT2300_INT_SEL_STATE_IS_STBY
*            CMT2300_INT_SEL_STATE_IS_FS
*            CMT2300_INT_SEL_STATE_IS_RX
*            CMT2300_INT_SEL_STATE_IS_TX
*            CMT2300_INT_SEL_LED
*            CMT2300_INT_SEL_TRX_ACTIVE
*            CMT2300_INT_SEL_PKT_DONE
* *********************************************************/
void Cmt2300_ConfigInterrupt(u8 nInt1Sel, u8 nInt2Sel)
{
    nInt1Sel &= CMT2300_MASK_INT1_SEL;
    nInt1Sel |= (~CMT2300_MASK_INT1_SEL) & Cmt2300_ReadReg(CMT2300_CUS_INT1_CTL);
    Cmt2300_WriteReg(CMT2300_CUS_INT1_CTL, nInt1Sel);

    nInt2Sel &= CMT2300_MASK_INT2_SEL;
    nInt2Sel |= (~CMT2300_MASK_INT2_SEL) & Cmt2300_ReadReg(CMT2300_CUS_INT2_CTL);
    Cmt2300_WriteReg(CMT2300_CUS_INT2_CTL, nInt2Sel);
}

/*! ********************************************************
* @name    Cmt2300_SetInterruptPolar
* @desc    Set the polarity of the interrupt.
* @param   bEnable(TRUE): active-high (default)
*          bEnable(FALSE): active-low
* *********************************************************/
void Cmt2300_SetInterruptPolar(BOOL bActiveHigh)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_INT1_CTL);

    if(bActiveHigh)
        tmp &= ~CMT2300_MASK_INT_POLAR;
    else
        tmp |= CMT2300_MASK_INT_POLAR;

    Cmt2300_WriteReg(CMT2300_CUS_INT1_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_SetFifoThreshold
* @desc    Set FIFO threshold.
* @param   nFifoThreshold
* *********************************************************/
void Cmt2300_SetFifoThreshold(u8 nFifoThreshold)
{ 
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_PKT29);
    
    tmp &= ~CMT2300_MASK_FIFO_TH;
    tmp |= nFifoThreshold & CMT2300_MASK_FIFO_TH;
    
    Cmt2300_WriteReg(CMT2300_CUS_PKT29, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableAntennaSwitch
* @desc    Enable antenna switch, output TX_ACTIVE/RX_ACTIVE
*          via GPIO1/GPIO2.
* @param   nMode 
*            0: RF_SWT1_EN=1, RF_SWT2_EN=0
*               GPIO1: RX_ACTIVE, GPIO2: TX_ACTIVE
*            1: RF_SWT1_EN=0, RF_SWT2_EN=1
*               GPIO1: RX_ACTIVE, GPIO2: ~RX_ACTIVE
* *********************************************************/
void Cmt2300_EnableAntennaSwitch(u8 nMode)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_INT1_CTL);

    if(0 == nMode) {
        tmp |= CMT2300_MASK_RF_SWT1_EN;
        tmp &= ~CMT2300_MASK_RF_SWT2_EN;
    }
    else if(1 == nMode) {
        tmp &= ~CMT2300_MASK_RF_SWT1_EN;
        tmp |= CMT2300_MASK_RF_SWT2_EN;
    }

    Cmt2300_WriteReg(CMT2300_CUS_INT1_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableInterrupt
* @desc    Enable interrupt.
* @param   nEnable 
*            CMT2300_MASK_SL_TMO_EN   |
*            CMT2300_MASK_RX_TMO_EN   |
*            CMT2300_MASK_TX_DONE_EN  |
*            CMT2300_MASK_PREAM_OK_EN |
*            CMT2300_MASK_SYNC_OK_EN  |
*            CMT2300_MASK_NODE_OK_EN  |
*            CMT2300_MASK_CRC_OK_EN   |
*            CMT2300_MASK_PKT_DONE_EN
* *********************************************************/
void Cmt2300_EnableInterrupt(u8 nEnable)
{
    Cmt2300_WriteReg(CMT2300_CUS_INT_EN, nEnable);
}

/*! ********************************************************
* @name    Cmt2300_EnableRxFifoAutoClear
* @desc    Auto clear Rx FIFO before entry Rx mode.
* @param   bEnable(TRUE): Enable it(default)
*          bEnable(FALSE): Disable it
* *********************************************************/
void Cmt2300_EnableRxFifoAutoClear(BOOL bEnable)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_CTL);

    if(bEnable)
        tmp &= ~CMT2300_MASK_FIFO_AUTO_CLR_DIS;
    else
        tmp |= CMT2300_MASK_FIFO_AUTO_CLR_DIS;

    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableFifoMerge
* @desc    Enable FIFO merge.
* @param   bEnable(TRUE): use a single 64-byte FIFO for either Tx or Rx
*          bEnable(FALSE): use a 32-byte FIFO for Tx and another 32-byte FIFO for Rx(default)
* *********************************************************/
void Cmt2300_EnableFifoMerge(BOOL bEnable)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_CTL);

    if(bEnable)
        tmp |= CMT2300_MASK_FIFO_MERGE_EN;
    else
        tmp &= ~CMT2300_MASK_FIFO_MERGE_EN;

    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableReadFifo
* @desc    Enable SPI to read the FIFO.
* *********************************************************/
void Cmt2300_EnableReadFifo(void)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_CTL);
    tmp &= ~CMT2300_MASK_SPI_FIFO_RD_WR_SEL; 
    tmp &= ~CMT2300_MASK_FIFO_RX_TX_SEL;
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableWriteFifo
* @desc    Enable SPI to write the FIFO.
* *********************************************************/
void Cmt2300_EnableWriteFifo(void)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_CTL);
    tmp |= CMT2300_MASK_SPI_FIFO_RD_WR_SEL;
    tmp |= CMT2300_MASK_FIFO_RX_TX_SEL;
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_RestoreFifo
* @desc    Restore the FIFO.
* *********************************************************/
void Cmt2300_RestoreFifo(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CLR, CMT2300_MASK_FIFO_RESTORE);
}

/*! ********************************************************
* @name    Cmt2300_ClearFifo
* @desc    Clear the Tx FIFO and Rx FIFO.
* @return  FIFO flags
*            CMT2300_MASK_RX_FIFO_FULL_FLG |
*            CMT2300_MASK_RX_FIFO_NMTY_FLG |
*            CMT2300_MASK_RX_FIFO_TH_FLG   |
*            CMT2300_MASK_RX_FIFO_OVF_FLG  |
*            CMT2300_MASK_TX_FIFO_FULL_FLG |
*            CMT2300_MASK_TX_FIFO_NMTY_FLG |
*            CMT2300_MASK_TX_FIFO_TH_FLG
* *********************************************************/
u8 Cmt2300_ClearFifo(void)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_FLAG);
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CLR, CMT2300_MASK_FIFO_CLR_RX | CMT2300_MASK_FIFO_CLR_TX);
    return tmp;
}
/*! ********************************************************
* @name    CMT2300A_RestoreFifo
* @desc    Restore the FIFO.
* *********************************************************/
void CMT2300A_RestoreFifo(void)
{
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CLR, CMT2300_MASK_FIFO_RESTORE);
}

/*! ********************************************************
* @name    CMT2300A_ClearTxFifo
* @desc    Clear the Tx FIFO.
* @return  FIFO flags
*            CMT2300A_MASK_RX_FIFO_FULL_FLG |
*            CMT2300A_MASK_RX_FIFO_NMTY_FLG |
*            CMT2300A_MASK_RX_FIFO_TH_FLG   |
*            CMT2300A_MASK_RX_FIFO_OVF_FLG  |
*            CMT2300A_MASK_TX_FIFO_FULL_FLG |
*            CMT2300A_MASK_TX_FIFO_NMTY_FLG |
*            CMT2300A_MASK_TX_FIFO_TH_FLG
**********************************************************/
u8 CMT2300A_ClearTxFifo(void)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_FLAG);
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CLR, CMT2300_MASK_FIFO_CLR_TX);
    return tmp;
}
/*! ********************************************************
* @name    CMT2300A_ClearRxFifo
* @desc    Clear the Rx FIFO.
* @return  FIFO flags
*            CMT2300A_MASK_RX_FIFO_FULL_FLG |
*            CMT2300A_MASK_RX_FIFO_NMTY_FLG |
*            CMT2300A_MASK_RX_FIFO_TH_FLG   |
*            CMT2300A_MASK_RX_FIFO_OVF_FLG  |
*            CMT2300A_MASK_TX_FIFO_FULL_FLG |
*            CMT2300A_MASK_TX_FIFO_NMTY_FLG |
*            CMT2300A_MASK_TX_FIFO_TH_FLG
* *********************************************************/
u8 CMT2300A_ClearRxFifo(void)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_FLAG);
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CLR, CMT2300_MASK_FIFO_CLR_RX);
    return tmp;
}

/*! ********************************************************
* @name    Cmt2300_ClearInterruptFlags
* @desc    Clear all interrupt flags.
* @return  Some interrupt flags
*            CMT2300_MASK_SL_TMO_EN    |
*            CMT2300_MASK_RX_TMO_EN    |
*            CMT2300_MASK_TX_DONE_EN   |
*            CMT2300_MASK_PREAM_OK_FLG |
*            CMT2300_MASK_SYNC_OK_FLG  |
*            CMT2300_MASK_NODE_OK_FLG  |
*            CMT2300_MASK_CRC_OK_FLG   |
*            CMT2300_MASK_PKT_OK_FLG
* *********************************************************/
u8 Cmt2300_ClearInterruptFlags(void)
{
    u8 nFlag1, nFlag2;
    u8 nClr1 = 0;
    u8 nClr2 = 0;
    u8 nRet  = 0;
    u8 nIntPolar;
    
    nIntPolar = Cmt2300_ReadReg(CMT2300_CUS_INT1_CTL);
    nIntPolar = (nIntPolar & CMT2300_MASK_INT_POLAR) ?1 :0;

    nFlag1 = Cmt2300_ReadReg(CMT2300_CUS_INT_FLAG);
    nFlag2 = Cmt2300_ReadReg(CMT2300_CUS_INT_CLR1);
    
    if(nIntPolar) {
        /* Interrupt flag active-low */
        nFlag1 = ~nFlag1;
        nFlag2 = ~nFlag2;
    }

    if(CMT2300_MASK_LBD_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_LBD_CLR;         /* Clear LBD_FLG */
    }

    if(CMT2300_MASK_COL_ERR_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_PKT_DONE_CLR;    /* Clear COL_ERR_FLG by PKT_DONE_CLR */
    }

    if(CMT2300_MASK_PKT_ERR_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_PKT_DONE_CLR;    /* Clear PKT_ERR_FLG by PKT_DONE_CLR */
    }

    if(CMT2300_MASK_PREAM_OK_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_PREAM_OK_CLR;    /* Clear PREAM_OK_FLG */
        nRet  |= CMT2300_MASK_PREAM_OK_FLG;    /* Return PREAM_OK_FLG */
    }

    if(CMT2300_MASK_SYNC_OK_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_SYNC_OK_CLR;    /* Clear SYNC_OK_FLG */
        nRet  |= CMT2300_MASK_SYNC_OK_FLG;    /* Return SYNC_OK_FLG */
    }

    if(CMT2300_MASK_NODE_OK_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_NODE_OK_CLR;    /* Clear NODE_OK_FLG */
        nRet  |= CMT2300_MASK_NODE_OK_FLG;    /* Return NODE_OK_FLG */
    }

    if(CMT2300_MASK_CRC_OK_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_CRC_OK_CLR;    /* Clear CRC_OK_FLG */
        nRet  |= CMT2300_MASK_CRC_OK_FLG;    /* Return CRC_OK_FLG */
    }

    if(CMT2300_MASK_PKT_OK_FLG & nFlag1) {
        nClr2 |= CMT2300_MASK_PKT_DONE_CLR;  /* Clear PKT_OK_FLG */
        nRet  |= CMT2300_MASK_PKT_OK_FLG;    /* Return PKT_OK_FLG */
    }    

    if(CMT2300_MASK_SL_TMO_FLG & nFlag2) {
        nClr1 |= CMT2300_MASK_SL_TMO_CLR;    /* Clear SL_TMO_FLG */
        nRet  |= CMT2300_MASK_SL_TMO_EN;     /* Return SL_TMO_FLG by SL_TMO_EN */
    }

    if(CMT2300_MASK_RX_TMO_FLG & nFlag2) {
        nClr1 |= CMT2300_MASK_RX_TMO_CLR;    /* Clear RX_TMO_FLG */
        nRet  |= CMT2300_MASK_RX_TMO_EN;     /* Return RX_TMO_FLG by RX_TMO_EN */
    }

    if(CMT2300_MASK_TX_DONE_FLG & nFlag2) {
        nClr1 |= CMT2300_MASK_TX_DONE_CLR;   /* Clear TX_DONE_FLG */
        nRet  |= CMT2300_MASK_TX_DONE_EN;    /* Return TX_DONE_FLG by TX_DONE_EN */
    }
    
    Cmt2300_WriteReg(CMT2300_CUS_INT_CLR1, nClr1);
    Cmt2300_WriteReg(CMT2300_CUS_INT_CLR2, nClr2);

    if(nIntPolar) {
        /* Interrupt flag active-low */
        nRet = ~nRet;
    }

    return nRet;
}

/*! ********************************************************
* @name    Cmt2300_ConfigTxDin
* @desc    Used to select whether to use GPIO1 or GPIO2 or GPIO3
*          as DIN in the direct mode. It only takes effect when 
*          call Cmt2300_EnableTxDin(TRUE) in the direct mode.
* @param   nDinSel
*            CMT2300_TX_DIN_SEL_GPIO1
*            CMT2300_TX_DIN_SEL_GPIO2
*            CMT2300_TX_DIN_SEL_GPIO3
* *********************************************************/
void Cmt2300_ConfigTxDin(u8 nDinSel)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_CTL);
    tmp &= ~CMT2300_MASK_TX_DIN_SEL;
    tmp |= nDinSel;
    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableTxDin
* @desc    Used to change GPIO1/GPIO2/GPIO3 between DOUT and DIN.
* @param   bEnable(TRUE): used as DIN
*          bEnable(FALSE): used as DOUT(default)
* *********************************************************/
void Cmt2300_EnableTxDin(BOOL bEnable)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_FIFO_CTL);

    if(bEnable)
        tmp |= CMT2300_MASK_TX_DIN_EN;
    else
        tmp &= ~CMT2300_MASK_TX_DIN_EN;

    Cmt2300_WriteReg(CMT2300_CUS_FIFO_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableTxDinInvert
* @desc    Used to invert DIN data in direct mode.
* @param   bEnable(TRUE): invert DIN
*          bEnable(FALSE): not invert DIN(default)
* *********************************************************/
void Cmt2300_EnableTxDinInvert(BOOL bEnable)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_INT2_CTL);

    if(bEnable)
        tmp |= CMT2300_MASK_TX_DIN_INV;
    else
        tmp &= ~CMT2300_MASK_TX_DIN_INV;

    Cmt2300_WriteReg(CMT2300_CUS_INT2_CTL, tmp);
}

/*! ********************************************************
* @name    Cmt2300_IsExist
* @desc    Chip indentify.
* @return  TRUE: chip is exist, FALSE: chip not found
* *********************************************************/
BOOL Cmt2300_IsExist(void)
{
    u8 back, dat;

    back = Cmt2300_ReadReg(CMT2300_CUS_PKT17);
    Cmt2300_WriteReg(CMT2300_CUS_PKT17, 0xAA);

    dat = Cmt2300_ReadReg(CMT2300_CUS_PKT17);
    Cmt2300_WriteReg(CMT2300_CUS_PKT17, back);

    if(0xAA==dat)
        return TRUE;
    else
        return FALSE;
}

/*! ********************************************************
* @name    Cmt2300_GetRssiCode
* @desc    Get RSSI code.
* @return  RSSI code
* *********************************************************/
u8 Cmt2300_GetRssiCode(void)
{
    return Cmt2300_ReadReg(CMT2300_CUS_RSSI_CODE);
}

/*! ********************************************************
* @name    Cmt2300_GetRssiDBm
* @desc    Get RSSI dBm.
* @return  dBm
* *********************************************************/
int Cmt2300_GetRssiDBm(void)
{
    return (int)Cmt2300_ReadReg(CMT2300_CUS_RSSI_DBM) - 128;
}

/*! ********************************************************
* @name    Cmt2300_SetFrequencyChannel
* @desc    This defines up to 255 frequency channel
*          for fast frequency hopping operation.
* @param   nChann: the frequency channel
* *********************************************************/
void Cmt2300_SetFrequencyChannel(u8 nChann)
{
    Cmt2300_WriteReg(CMT2300_CUS_FREQ_CHNL, nChann);
}

/*! ********************************************************
* @name    Cmt2300_SetFrequencyStep
* @desc    This defines the frequency channel step size 
*          for fast frequency hopping operation. 
*          One step size is 2.5 kHz.
* @param   nOffset: the frequency step
* *********************************************************/
void Cmt2300_SetFrequencyStep(u8 nOffset)
{
    Cmt2300_WriteReg(CMT2300_CUS_FREQ_OFS, nOffset);
}

/*! ********************************************************
* @name    Cmt2300_SetPayloadLength
* @desc    Set payload length.
* @param   nLength
* *********************************************************/
void Cmt2300_SetPayloadLength(u16 nLength)
{ 
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_PKT14);
    
    tmp &= ~CMT2300_MASK_PAYLOAD_LENG_10_8;
    tmp |= (nLength >> 4) & CMT2300_MASK_PAYLOAD_LENG_10_8;
    Cmt2300_WriteReg(CMT2300_CUS_PKT14, tmp);
    
    tmp = nLength & CMT2300_MASK_PAYLOAD_LENG_7_0;
    Cmt2300_WriteReg(CMT2300_CUS_PKT15, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableLfosc
* @desc    If you need use sleep timer, you should enable LFOSC.
* @param   bEnable(TRUE): Enable it(default)
*          bEnable(FALSE): Disable it
* *********************************************************/
void Cmt2300_EnableLfosc(BOOL bEnable)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_SYS2);
    
    if(bEnable) {
        tmp |= CMT2300_MASK_LFOSC_RECAL_EN;
        tmp |= CMT2300_MASK_LFOSC_CAL1_EN;
        tmp |= CMT2300_MASK_LFOSC_CAL2_EN;
    }
    else {
        tmp &= ~CMT2300_MASK_LFOSC_RECAL_EN;
        tmp &= ~CMT2300_MASK_LFOSC_CAL1_EN;
        tmp &= ~CMT2300_MASK_LFOSC_CAL2_EN;
    }
    
    Cmt2300_WriteReg(CMT2300_CUS_SYS2, tmp);
}

/*! ********************************************************
* @name    Cmt2300_EnableLfoscOutput
* @desc    LFOSC clock is output via GPIO3.
* @param   bEnable(TRUE): Enable it
*          bEnable(FALSE): Disable it(default)
* *********************************************************/
void Cmt2300_EnableLfoscOutput(BOOL bEnable)
{
    u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_INT2_CTL);
    
    if(bEnable)
        tmp |= CMT2300_MASK_LFOSC_OUT_EN;
    else
        tmp &= ~CMT2300_MASK_LFOSC_OUT_EN;
    
    Cmt2300_WriteReg(CMT2300_CUS_INT2_CTL, tmp);
}
/**
  * @brief  Set Synchronized word length
  * @param  The param range is 1 to 8 bytes
  * @retval None
  */
void Cmt2300_SetSyncLength(u8 nLength)
{
	u8 tmp = Cmt2300_ReadReg(CMT2300_CUS_PKT5);
	
	nLength--;
	tmp &= ~CMT2300_MASK_SYNC_SIZE;  
	tmp |= (nLength << 1) & CMT2300_MASK_SYNC_SIZE;
	
	Cmt2300_WriteReg(CMT2300_CUS_PKT5,tmp);
//        tmp = Cmt2300_ReadReg(CMT2300_CUS_PKT5);
}

/**
  * @brief  Write the Synchronized Word ,the default length is 4bytes.If length more than 4 bytes need override this function
  * @param  nLength  should be same as Cmt2300_SetSyncLength param
  *         nWord    
  * @retval None
  */
void Cmt2300_SetSynWord(u32 nWord)
{
  Cmt2300_WriteReg(CMT2300_CUS_PKT13,(u8)(nWord>>24));//(u8)nWord>>24
  Cmt2300_WriteReg(CMT2300_CUS_PKT12,(u8)(nWord>>16));//(u8)nWord>>16
  Cmt2300_WriteReg(CMT2300_CUS_PKT11,(u8)(nWord>>8));//(u8)nWord>>8
  Cmt2300_WriteReg(CMT2300_CUS_PKT10,(u8)nWord);
}
/**
  * @brief  Read synchronized word only 4 bytes
  * @param  None
  * @retval None
  */
void Cmt2300_ReadSynWord(u32 * syn)
{
//	u32 syn;
	*syn=Cmt2300_ReadReg(CMT2300_CUS_PKT13)<<24;
	*syn|=(Cmt2300_ReadReg(CMT2300_CUS_PKT12)<<16);
	*syn|=(Cmt2300_ReadReg(CMT2300_CUS_PKT11)<<8);
	*syn|=Cmt2300_ReadReg(CMT2300_CUS_PKT10);
//  return syn;
}
/**
  * @brief  Set resend time in TX mode
  * @param  It's range of 0~255，means 1~256 packets to resend
  * @retval None
  */
void Cmt2300_SetResendTime(u8 nLength)
{
 Cmt2300_WriteReg(CMT2300_CUS_PKT27,nLength);
}

/*! ********************************************************
* @name    Cmt2300_Init
* @desc    Initialize chip status.
* *********************************************************/
void Cmt2300_Init(void)
{
    u8 tmp;

    Cmt2300_SoftReset();
    delay_ms(20);
    
    Cmt2300_GoStby();

    tmp  = Cmt2300_ReadReg(CMT2300_CUS_MODE_STA);
    tmp |= CMT2300_MASK_CFG_RETAIN;         /* Enable CFG_RETAIN */
    tmp &= ~CMT2300_MASK_RSTN_IN_EN;        /* Disable RSTN_IN */
    Cmt2300_WriteReg(CMT2300_CUS_MODE_STA, tmp);
    
    Cmt2300_EnableLfosc(FALSE);             /* Diable LFOSC */

    Cmt2300_ClearInterruptFlags();
}

/*! ********************************************************
* @name    Cmt2300_ConfigRegBank
* @desc    Config one register bank.
* *********************************************************/
BOOL Cmt2300_ConfigRegBank(u8 base_addr, const u8 bank[], u8 len)
{
    u8 i;
    for(i=0; i<len; i++)
        Cmt2300_WriteReg(i+base_addr, bank[i]);

    return TRUE;
}

void RF_Init(void)
{
		Cmt2300_InitGpio(); //引脚及模拟spi初始化
		Cmt2300_Init();
	
		//write the configuration data form RFPDK
		Cmt2300_ConfigRegBank(0x00,CMTBank,12);
		Cmt2300_ConfigRegBank(0x0C,SystemBank,12);
		Cmt2300_ConfigRegBank(0x18,FrequencyBank,8);
		Cmt2300_ConfigRegBank(0x20,DataRateBank,24);
		Cmt2300_ConfigRegBank(0x38,BasebandBank,29);
		Cmt2300_ConfigRegBank(0x55,TXBank,11);

//		Cmt2300_EnableAntennaSwitch(0);
    Cmt2300_ConfigGpio(CMT2300_GPIO3_SEL_INT2+CMT2300_GPIO1_SEL_DIN+CMT2300_GPIO2_SEL_DOUT);
//	  Cmt2300_GoSleep();
	  Cmt2300_ClearFifo();
	
		Cmt2300_EnableTxDin(TRUE); //第一步
    Cmt2300_ConfigTxDin(CMT2300_TX_DIN_SEL_GPIO1); //第二步
	
//		while(FALSE==Cmt2300_GoSleep())  //17.12.25
//			delay_ms(10);
		CMT2300_ForceRX();
// 		EXTIX_Init();
}

