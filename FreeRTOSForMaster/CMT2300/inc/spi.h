//#include "typedefs.h"
//#include "stm32f10x_conf.h"
#ifndef __SPI_H
#define __SPI_H
#ifdef __cplusplus
 extern "C" {
#endif
//#include "stm32f103xb.h"
//#include "stm32f1xx_hal_gpio.h"
#include "sys.h"

/* Exported constants --------------------------------------------------------*/

/** @defgroup GPIO_Exported_Constants GPIO Exported Constants
  * @{
  */

/** @defgroup GPIO_pins_define GPIO pins define
  * @{
  */
#define GPIO_PIN_0                 ((uint16_t)0x0001)  /* Pin 0 selected    */
#define GPIO_PIN_1                 ((uint16_t)0x0002)  /* Pin 1 selected    */
#define GPIO_PIN_2                 ((uint16_t)0x0004)  /* Pin 2 selected    */
#define GPIO_PIN_3                 ((uint16_t)0x0008)  /* Pin 3 selected    */
#define GPIO_PIN_4                 ((uint16_t)0x0010)  /* Pin 4 selected    */
#define GPIO_PIN_5                 ((uint16_t)0x0020)  /* Pin 5 selected    */
#define GPIO_PIN_6                 ((uint16_t)0x0040)  /* Pin 6 selected    */
#define GPIO_PIN_7                 ((uint16_t)0x0080)  /* Pin 7 selected    */
#define GPIO_PIN_8                 ((uint16_t)0x0100)  /* Pin 8 selected    */
#define GPIO_PIN_9                 ((uint16_t)0x0200)  /* Pin 9 selected    */
#define GPIO_PIN_10                ((uint16_t)0x0400)  /* Pin 10 selected   */
#define GPIO_PIN_11                ((uint16_t)0x0800)  /* Pin 11 selected   */
#define GPIO_PIN_12                ((uint16_t)0x1000)  /* Pin 12 selected   */
#define GPIO_PIN_13                ((uint16_t)0x2000)  /* Pin 13 selected   */
#define GPIO_PIN_14                ((uint16_t)0x4000)  /* Pin 14 selected   */
#define GPIO_PIN_15                ((uint16_t)0x8000)  /* Pin 15 selected   */
#define GPIO_PIN_All               ((uint16_t)0xFFFF)  /* All pins selected */

#define GPIO_PIN_MASK              0x0000FFFFU /* PIN mask for assert test */
	 
#define CMT_CSB_GPIO                GPIOA
#define CMT_CSB_GPIO_PIN            GPIO_PIN_0

#define CMT_FCSB_GPIO               GPIOA
#define CMT_FCSB_GPIO_PIN           GPIO_PIN_2

#define CMT_SCL_GPIO                GPIOA
#define CMT_SCL_GPIO_PIN            GPIO_PIN_1

#define CMT_SDA_GPIO                GPIOD
#define CMT_SDA_GPIO_PIN            GPIO_PIN_1

#define CMT_GPIO1_GPIO              GPIOA
#define CMT_GPIO1_GPIO_PIN          GPIO_PIN_5

#define CMT_GPIO2_GPIO              GPIOA
#define CMT_GPIO2_GPIO_PIN          GPIO_PIN_6

#define CMT_GPIO3_GPIO              GPIOA         
#define CMT_GPIO3_GPIO_PIN          GPIO_PIN_7

//模块上的天线引脚
#define CMT_RX_ANT                  GPIOA
#define CMT_RX_ANT_PIN              GPIO_PIN_4

#define CMT_TX_ANT                  GPIOA
#define CMT_TX_ANT_PIN              GPIO_PIN_3
                 

#define cmt_spi3_csb_out()      GPIO_Pin_Setting(CMT_CSB_GPIO,	 	CMT_CSB_GPIO_PIN, 	GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)
#define cmt_spi3_fcsb_out()     GPIO_Pin_Setting(CMT_FCSB_GPIO,		CMT_FCSB_GPIO_PIN, 	GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)
#define cmt_spi3_scl_out()      GPIO_Pin_Setting(CMT_SCL_GPIO,		CMT_SCL_GPIO_PIN, 	GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)
#define cmt_spi3_sda_out()      GPIO_Pin_Setting(CMT_SDA_GPIO,	 	CMT_SDA_GPIO_PIN, 	GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)
#define cmt_spi3_sda_in()       GPIO_Pin_Setting(CMT_SDA_GPIO, 		CMT_SDA_GPIO_PIN, 	GPIO_SPEED_FREQ_HIGH, GPIO_MODE_INPUT		 ,GPIO_NOPULL)

//#define Cmt2300_SetGpio1In()    GPIO_Pin_Setting(CMT_GPIO1_GPIO, CMT_GPIO1_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
//#define Cmt2300_SetGpio2In()    GPIO_Pin_Setting(CMT_GPIO2_GPIO, CMT_GPIO2_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
//#define Cmt2300_SetGpio3In()    GPIO_Pin_Setting(CMT_GPIO3_GPIO, CMT_GPIO3_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)

//天线对应的MCU IO初始化 
#define cmt_rxant_setpin()      GPIO_Pin_Setting(CMT_RX_ANT,CMT_RX_ANT_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)
#define cmt_txant_setpin()      GPIO_Pin_Setting(CMT_TX_ANT, CMT_TX_ANT_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)

#define cmt_spi3_csb_1()        HAL_GPIO_WritePin(CMT_CSB_GPIO,	CMT_CSB_GPIO_PIN,	GPIO_PIN_SET)//GPIO_SetBits(CMT_CSB_GPIO,CMT_CSB_GPIO_PIN)
#define cmt_spi3_csb_0()        HAL_GPIO_WritePin(CMT_CSB_GPIO,	CMT_CSB_GPIO_PIN,	GPIO_PIN_RESET)

#define cmt_spi3_fcsb_1()       HAL_GPIO_WritePin(CMT_FCSB_GPIO,CMT_FCSB_GPIO_PIN,	GPIO_PIN_SET)
#define cmt_spi3_fcsb_0()       HAL_GPIO_WritePin(CMT_FCSB_GPIO,CMT_FCSB_GPIO_PIN,	GPIO_PIN_RESET)
    
#define cmt_spi3_scl_1()        HAL_GPIO_WritePin(CMT_SCL_GPIO,	CMT_SCL_GPIO_PIN,	GPIO_PIN_SET) 
#define cmt_spi3_scl_0()        HAL_GPIO_WritePin(CMT_SCL_GPIO,	CMT_SCL_GPIO_PIN,	GPIO_PIN_RESET)

#define cmt_spi3_sda_1()        HAL_GPIO_WritePin(CMT_SDA_GPIO,	CMT_SDA_GPIO_PIN,	GPIO_PIN_SET)
#define cmt_spi3_sda_0()        HAL_GPIO_WritePin(CMT_SDA_GPIO,	CMT_SDA_GPIO_PIN,	GPIO_PIN_RESET)

#define cmt_rx_ant_1()          HAL_GPIO_WritePin(CMT_RX_ANT,		CMT_RX_ANT_PIN,	GPIO_PIN_SET)
#define cmt_rx_ant_0()          HAL_GPIO_WritePin(CMT_RX_ANT,		CMT_RX_ANT_PIN,	GPIO_PIN_RESET)

#define cmt_tx_ant_1()          HAL_GPIO_WritePin(CMT_TX_ANT,		CMT_TX_ANT_PIN,	GPIO_PIN_SET)
#define cmt_tx_ant_0()          HAL_GPIO_WritePin(CMT_TX_ANT,		CMT_TX_ANT_PIN,	GPIO_PIN_RESET)

#define cmt_spi3_sda_read()     HAL_GPIO_ReadPin(CMT_SDA_GPIO,CMT_SDA_GPIO_PIN)

#define ANT_RX()  {cmt_rx_ant_0();cmt_tx_ant_1();}
#define ANT_TX()  {cmt_rx_ant_1();cmt_tx_ant_0();}

#define SET_GPIO_OUT(x)             GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)
#define SET_GPIO_IN_RISFAL(x)       GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_MODE_IT_RISING_FALLING    ,GPIO_NOPULL)
#define SET_GPIO_IN_RIS(x)					GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_MODE_IT_RISING    ,GPIO_NOPULL)
#define SET_GPIO_IN(x)							GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_MODE_INPUT    ,GPIO_NOPULL)
#define SET_GPIO_OD(x)              GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_Mode_Out_OD)
#define SET_GPIO_AIN(x)             GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_Mode_AIN)
#define SET_GPIO_AFOUT(x)           GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_MODE_OUTPUT_PP,GPIO_PULLUP)
#define SET_GPIO_AFOD(x)            GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_HIGH, GPIO_Mode_AF_OD)
#define SET_GPIO_H(x)               HAL_GPIO_WritePin(x,x##_PIN,GPIO_PIN_SET)//(x->BSRR = x##_PIN) //GPIO_SetBits(x, x##_PIN)
#define SET_GPIO_L(x)               HAL_GPIO_WritePin(x,x##_PIN,GPIO_PIN_RESET)//(x->BRR  = x##_PIN) //GPIO_ResetBits(x, x##_PIN)##表示宏结合 结合过后再展开
#define READ_GPIO_PIN(x)            HAL_GPIO_ReadPin(x,x##_PIN)//(((x->IDR & x##_PIN)!=Bit_RESET) ?1 :0) //GPIO_ReadInputDataBit(x, x##_PIN)

void cmt_spi3_delay(void);
void cmt_spi3_delay_us(void);

void cmt_spi3_init(void);

void cmt_spi3_send(u8 data8);
u8 cmt_spi3_recv(void);

void cmt_spi3_write(u8 addr, u8 dat);
void cmt_spi3_read(u8 addr, u8* p_dat);

void cmt_spi3_write_fifo(const u8* p_buf, u16 len);//
void cmt_spi3_read_fifo(u8* p_buf, u16 len);
#ifdef __cplusplus
}
#endif
#endif
