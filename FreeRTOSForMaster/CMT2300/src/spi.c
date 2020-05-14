#include "spi.h"
//#include "portmacro.h"
#include "FreeRTOS.h"
void GPIO_Pin_Setting(GPIO_TypeDef *gpio, uint16_t nPin, u32 speed, u32 mode,u32 pull)
{
		GPIO_InitTypeDef GPIO_InitStructure;

		GPIO_InitStructure.Pin 	= nPin;
		GPIO_InitStructure.Mode = mode;
		GPIO_InitStructure.Pull	= pull;
		GPIO_InitStructure.Speed=	speed;
		HAL_GPIO_Init(gpio, &GPIO_InitStructure);
}

/* ************************************************************************ */
void cmt_spi3_delay(void)
{
    u32 n = 7;
    while(n--);
}

void cmt_spi3_delay_us(void)
{
    u16 n = 8;
    while(n--);
}

void cmt_spi3_init(void)
{
		portENTER_CRITICAL(); //进入临界段
    cmt_spi3_csb_1();   //CSB拉高
    cmt_spi3_csb_out(); //设置成输出
    cmt_spi3_csb_1();   /* CSB has an internal pull-up resistor */
    
    cmt_spi3_scl_0();   //时钟
    cmt_spi3_scl_out();
    cmt_spi3_scl_0();   /* SCL has an internal pull-down resistor */
    
    cmt_spi3_sda_1();
    cmt_spi3_sda_out();
    cmt_spi3_sda_1();
    
    cmt_spi3_fcsb_1();
    cmt_spi3_fcsb_out();
    cmt_spi3_fcsb_1();  /* FCSB has an internal pull-up resistor */

    cmt_spi3_delay();
	
		portEXIT_CRITICAL(); //退出临界段
}

void cmt_spi3_send(u8 data8)
{
    u8 i;
		portENTER_CRITICAL();
    for(i=0; i<8; i++)
    {
        cmt_spi3_scl_0();

        /* Send byte on the rising edge of SCL */
        if(data8 & 0x80)
            cmt_spi3_sda_1();
        else            
            cmt_spi3_sda_0();

        cmt_spi3_delay();

        data8 <<= 1;
        cmt_spi3_scl_1();
        cmt_spi3_delay();
    }
		portEXIT_CRITICAL();
}

u8 cmt_spi3_recv(void)
{
    u8 i;
    u8 data8 = 0xFF;
		portENTER_CRITICAL();
    for(i=0; i<8; i++)
    {
        cmt_spi3_scl_0();
        cmt_spi3_delay();
        data8 <<= 1;

        cmt_spi3_scl_1();

        /* Read byte on the rising edge of SCL */
        if(cmt_spi3_sda_read())
            data8 |= 0x01;
        else
            data8 &= ~0x01;

        cmt_spi3_delay();
    }
		portEXIT_CRITICAL();
    return data8;
		
}

void cmt_spi3_write(u8 addr, u8 dat)
{
//		portENTER_CRITICAL();
    cmt_spi3_sda_1();
    cmt_spi3_sda_out();

    cmt_spi3_scl_0();
    cmt_spi3_scl_out();
    cmt_spi3_scl_0(); 

    cmt_spi3_fcsb_1();
    cmt_spi3_fcsb_out();
    cmt_spi3_fcsb_1();

    cmt_spi3_csb_0();

    /* > 0.5 SCL cycle */
    cmt_spi3_delay();
    cmt_spi3_delay();

    /* r/w = 0 */
    cmt_spi3_send(addr&0x7F);

    cmt_spi3_send(dat);

    cmt_spi3_scl_0();

    /* > 0.5 SCL cycle */
    cmt_spi3_delay();
    cmt_spi3_delay();

    cmt_spi3_csb_1();
    
    cmt_spi3_sda_1();
    cmt_spi3_sda_in();
    
    cmt_spi3_fcsb_1();   
//		portEXIT_CRITICAL();
}

void cmt_spi3_read(u8 addr, u8* p_dat)
{
		portENTER_CRITICAL();
    cmt_spi3_sda_1();
    cmt_spi3_sda_out();

    cmt_spi3_scl_0();
    cmt_spi3_scl_out();
    cmt_spi3_scl_0(); 

    cmt_spi3_fcsb_1();
    cmt_spi3_fcsb_out();
    cmt_spi3_fcsb_1();

    cmt_spi3_csb_0();

    /* > 0.5 SCL cycle */
    cmt_spi3_delay();
    cmt_spi3_delay();

    /* r/w = 1 */
    cmt_spi3_send(addr|0x80);

    /* Must set SDA to input before the falling edge of SCL */
    cmt_spi3_sda_in();
    
    *p_dat = cmt_spi3_recv();

    cmt_spi3_scl_0();

    /* > 0.5 SCL cycle */
    cmt_spi3_delay();
    cmt_spi3_delay();

    cmt_spi3_csb_1();
    
    cmt_spi3_sda_1();
    cmt_spi3_sda_in();
    
    cmt_spi3_fcsb_1();
		portEXIT_CRITICAL();
}

void cmt_spi3_write_fifo(const u8* p_buf, u16 len)//
{
    u16 i;
		portENTER_CRITICAL();
    cmt_spi3_fcsb_1();
    cmt_spi3_fcsb_out();
    cmt_spi3_fcsb_1();

    cmt_spi3_csb_1();
    cmt_spi3_csb_out();
    cmt_spi3_csb_1();

    cmt_spi3_scl_0();
    cmt_spi3_scl_out();
    cmt_spi3_scl_0();

    cmt_spi3_sda_out();

    for(i=0; i<len; i++)
    {
        cmt_spi3_fcsb_0();

        /* > 1 SCL cycle */
        cmt_spi3_delay();
        cmt_spi3_delay();

        cmt_spi3_send(p_buf[i]);

        cmt_spi3_scl_0();

        /* > 2 us */
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();

        cmt_spi3_fcsb_1();

        /* > 4 us */
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
    }

    cmt_spi3_sda_in();
    
    cmt_spi3_fcsb_1();
		portEXIT_CRITICAL();
}

void cmt_spi3_read_fifo(u8* p_buf, u16 len)
{
    u16 i;
		portENTER_CRITICAL();
    cmt_spi3_fcsb_1();
    cmt_spi3_fcsb_out();
    cmt_spi3_fcsb_1();

    cmt_spi3_csb_1();
    cmt_spi3_csb_out();
    cmt_spi3_csb_1();

    cmt_spi3_scl_0();
    cmt_spi3_scl_out();
    cmt_spi3_scl_0();

    cmt_spi3_sda_in();

    for(i=0; i<len; i++)
    {
        cmt_spi3_fcsb_0();

        /* > 1 SCL cycle */
        cmt_spi3_delay();
        cmt_spi3_delay();

        p_buf[i] = cmt_spi3_recv();

        cmt_spi3_scl_0();

        /* > 2 us */
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();

        cmt_spi3_fcsb_1();

        /* > 4 us */
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
        cmt_spi3_delay_us();
    }

    cmt_spi3_sda_in();
    
    cmt_spi3_fcsb_1();
		portEXIT_CRITICAL();
}

