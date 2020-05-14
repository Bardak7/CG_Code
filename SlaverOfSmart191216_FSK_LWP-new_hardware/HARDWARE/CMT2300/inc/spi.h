#include "typedefs.h"
#include "stm32f10x_conf.h"

#define CMT_CSB_GPIO                GPIOA
#define CMT_CSB_GPIO_PIN            GPIO_Pin_11

#define CMT_FCSB_GPIO               GPIOA
#define CMT_FCSB_GPIO_PIN           GPIO_Pin_15

#define CMT_SCL_GPIO                GPIOA
#define CMT_SCL_GPIO_PIN            GPIO_Pin_12

#define CMT_SDA_GPIO                GPIOA
#define CMT_SDA_GPIO_PIN            GPIO_Pin_10

#define CMT_GPIO1_GPIO              GPIOB
#define CMT_GPIO1_GPIO_PIN          GPIO_Pin_5

#define CMT_GPIO2_GPIO              GPIOB
#define CMT_GPIO2_GPIO_PIN          GPIO_Pin_6

#define CMT_GPIO3_GPIO              GPIOB         
#define CMT_GPIO3_GPIO_PIN          GPIO_Pin_7
//模块上的天线引脚
#define CMT_RX_ANT                  GPIOB
#define CMT_RX_ANT_PIN              GPIO_Pin_4

#define CMT_TX_ANT                  GPIOB
#define CMT_TX_ANT_PIN              GPIO_Pin_3

#define cmt_spi3_csb_out()      GPIO_Pin_Setting(CMT_CSB_GPIO, CMT_CSB_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_fcsb_out()     GPIO_Pin_Setting(CMT_FCSB_GPIO, CMT_FCSB_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_scl_out()      GPIO_Pin_Setting(CMT_SCL_GPIO, CMT_SCL_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_sda_out()      GPIO_Pin_Setting(CMT_SDA_GPIO, CMT_SDA_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_sda_in()       GPIO_Pin_Setting(CMT_SDA_GPIO, CMT_SDA_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)

#define Cmt2300_SetGpio1In()    GPIO_Pin_Setting(CMT_GPIO1_GPIO, CMT_GPIO1_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
#define Cmt2300_SetGpio2In()    GPIO_Pin_Setting(CMT_GPIO2_GPIO, CMT_GPIO2_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
#define Cmt2300_SetGpio3In()    GPIO_Pin_Setting(CMT_GPIO3_GPIO, CMT_GPIO3_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)

#define cmt_rx_ant()            GPIO_Pin_Setting(CMT_RX_ANT, CMT_RX_ANT_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_tx_ant()            GPIO_Pin_Setting(CMT_TX_ANT, CMT_TX_ANT_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)

#define cmt_spi3_csb_1()        GPIO_SetBits(CMT_CSB_GPIO,CMT_CSB_GPIO_PIN)
#define cmt_spi3_csb_0()        GPIO_ResetBits(CMT_CSB_GPIO,CMT_CSB_GPIO_PIN)

#define cmt_spi3_fcsb_1()       GPIO_SetBits(CMT_FCSB_GPIO,CMT_FCSB_GPIO_PIN)
#define cmt_spi3_fcsb_0()       GPIO_ResetBits(CMT_FCSB_GPIO,CMT_FCSB_GPIO_PIN)
    
#define cmt_spi3_scl_1()        GPIO_SetBits(CMT_SCL_GPIO,CMT_SCL_GPIO_PIN)
#define cmt_spi3_scl_0()        GPIO_ResetBits(CMT_SCL_GPIO,CMT_SCL_GPIO_PIN)

#define cmt_spi3_sda_1()        GPIO_SetBits(CMT_SDA_GPIO,CMT_SDA_GPIO_PIN)
#define cmt_spi3_sda_0()        GPIO_ResetBits(CMT_SDA_GPIO,CMT_SDA_GPIO_PIN)

#define cmt_rx_ant_1()          GPIO_SetBits(CMT_RX_ANT,CMT_RX_ANT_PIN)
#define cmt_rx_ant_0()          GPIO_ResetBits(CMT_RX_ANT,CMT_RX_ANT_PIN)

#define cmt_tx_ant_1()          GPIO_SetBits(CMT_TX_ANT,CMT_TX_ANT_PIN)
#define cmt_tx_ant_0()          GPIO_ResetBits(CMT_TX_ANT,CMT_TX_ANT_PIN)

#define cmt_spi3_sda_read()     GPIO_ReadInputDataBit(CMT_SDA_GPIO,CMT_SDA_GPIO_PIN)

#define ANT_RX()  {cmt_rx_ant_0();cmt_tx_ant_1();}
#define ANT_TX()  {cmt_rx_ant_1();cmt_tx_ant_0();}

#define SET_GPIO_OUT(x)             GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define SET_GPIO_IN(x)              GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
#define SET_GPIO_OD(x)              GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_OD)
#define SET_GPIO_AIN(x)             GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_AIN)
#define SET_GPIO_AFOUT(x)           GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_AF_PP)
#define SET_GPIO_AFOD(x)            GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_AF_OD)
#define SET_GPIO_H(x)               (x->BSRR = x##_PIN) //GPIO_SetBits(x, x##_PIN)
#define SET_GPIO_L(x)               (x->BRR  = x##_PIN) //GPIO_ResetBits(x, x##_PIN)##表示宏结合 结合过后再展开
#define READ_GPIO_PIN(x)            (((x->IDR & x##_PIN)!=Bit_RESET) ?1 :0) //GPIO_ReadInputDataBit(x, x##_PIN)


void cmt_spi3_delay(void);
void cmt_spi3_delay_us(void);

void cmt_spi3_init(void);

void cmt_spi3_send(u8 data8);
u8 cmt_spi3_recv(void);

void cmt_spi3_write(u8 addr, u8 dat);
void cmt_spi3_read(u8 addr, u8* p_dat);

void cmt_spi3_write_fifo(const u8* p_buf, u16 len);
void cmt_spi3_read_fifo(u8* p_buf, u16 len);

