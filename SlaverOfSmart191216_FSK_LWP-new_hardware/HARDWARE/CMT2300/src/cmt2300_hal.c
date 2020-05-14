#include "cmt2300_hal.h"
#include "spi.h"

extern void GPIO_Pin_Setting(GPIO_TypeDef *gpio, uint16_t nPin, GPIOSpeed_TypeDef speed, GPIOMode_TypeDef mode);
/*! ********************************************************
* @name    Cmt2300_InitGpio
* @desc    Initializes the CMT2300 interface GPIOs.
* *********************************************************/
void Cmt2300_InitGpio(void)
{
		//Cmt2300_SetGpio1In();
		//Cmt2300_SetGpio2In();
		//Cmt2300_SetGpio3In();
//	CMT2300A_SetRF433Out()  
	  SET_GPIO_OUT(CMT_GPIO1_GPIO); //433OOK输出
//	  cmt_rx_ant();
//    cmt_tx_ant(); 	
		cmt_spi3_init();
}
/*! ********************************************************
* @name    CMT2300A_InitGpio
* @desc    Initializes the CMT2300A interface GPIOs. for fsk
* *********************************************************/
void CMT2300A_InitGpio(void)
{
    CMT2300A_SetGpio1In();
    CMT2300A_SetGpio2In();
    CMT2300A_SetGpio3In();
	  //初始化天线引脚 RX时 RX_PIN = 0  TX_PIN = 1 反之亦然
	  cmt_rx_ant();
    cmt_tx_ant();
	
    cmt_spi3_init();
}
/*! ********************************************************
* @name    Cmt2300_ReadReg
* @desc    Read the CMT2300 register at the specified address.
* @param   addr: register address
* @return  Register value
* *********************************************************/
u8 Cmt2300_ReadReg(u8 addr)
{
    u8 dat = 0xFF;
    cmt_spi3_read(addr, &dat);
	
    return dat;
}

/*! ********************************************************
* @name    Cmt2300_WriteReg
* @desc    Write the CMT2300 register at the specified address.
* @param   addr: register address
*          dat: register value
* *********************************************************/
void Cmt2300_WriteReg(u8 addr, u8 dat)
{
    cmt_spi3_write(addr, dat);
}

/*! ********************************************************
* @name    Cmt2300_ReadFifo
* @desc    Reads the contents of the CMT2300 FIFO.
* @param   buf: buffer where to copy the FIFO read data
*          len: number of bytes to be read from the FIFO
* *********************************************************/
void Cmt2300_ReadFifo(u8 buf[], u16 len)
{
    cmt_spi3_read_fifo(buf, len);
}

/*! ********************************************************
* @name    Cmt2300_WriteFifo
* @desc    Writes the buffer contents to the CMT2300 FIFO.
* @param   buf: buffer containing data to be put on the FIFO
*          len: number of bytes to be written to the FIFO
* *********************************************************/
void Cmt2300_WriteFifo(const u8 buf[], u16 len)
{
    cmt_spi3_write_fifo(buf, len);
}

