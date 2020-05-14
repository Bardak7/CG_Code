//#include "typedefs.h"
//#include "stm32f10x_conf.h"
#include "stm8s.h"

#define CMT_CSB_GPIO                GPIOC
#define CMT_CSB_GPIO_PIN            GPIO_PIN_4

#define CMT_FCSB_GPIO               GPIOB
#define CMT_FCSB_GPIO_PIN           GPIO_PIN_4

#define CMT_SCL_GPIO                GPIOC
#define CMT_SCL_GPIO_PIN            GPIO_PIN_3

#define CMT_SDA_GPIO                GPIOC
#define CMT_SDA_GPIO_PIN            GPIO_PIN_5

#define CMT_GPIO1_GPIO              GPIOA
#define CMT_GPIO1_GPIO_PIN          GPIO_PIN_1

#define CMT_GPIO2_GPIO              GPIOC
#define CMT_GPIO2_GPIO_PIN          GPIO_PIN_7

#define CMT_GPIO3_GPIO              GPIOC         
#define CMT_GPIO3_GPIO_PIN          GPIO_PIN_6
//模块上的天线引脚
#define CMT_RX_ANT                  GPIOA
#define CMT_RX_ANT_PIN              GPIO_PIN_2

#define CMT_TX_ANT                  GPIOA
#define CMT_TX_ANT_PIN              GPIO_PIN_3
                 

#define cmt_spi3_csb_out()      SET_GPIO_OUT(CMT_CSB_GPIO)//GPIO_Pin_Setting(CMT_CSB_GPIO, CMT_CSB_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_fcsb_out()     SET_GPIO_OUT(CMT_FCSB_GPIO)//GPIO_Pin_Setting(CMT_FCSB_GPIO, CMT_FCSB_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_scl_out()      SET_GPIO_OUT(CMT_SCL_GPIO)//GPIO_Pin_Setting(CMT_SCL_GPIO, CMT_SCL_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_sda_out()      SET_GPIO_OUT(CMT_SDA_GPIO)//GPIO_Pin_Setting(CMT_SDA_GPIO, CMT_SDA_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_spi3_sda_in()       SET_GPIO_IN_NOINT(CMT_SDA_GPIO)//GPIO_Pin_Setting(CMT_SDA_GPIO, CMT_SDA_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)

//#define Cmt2300_SetGpio1In()    GPIO_Pin_Setting(CMT_GPIO1_GPIO, CMT_GPIO1_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
//#define Cmt2300_SetGpio2In()    GPIO_Pin_Setting(CMT_GPIO2_GPIO, CMT_GPIO2_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
//#define Cmt2300_SetGpio3In()    GPIO_Pin_Setting(CMT_GPIO3_GPIO, CMT_GPIO3_GPIO_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)

//天线对应的MCU IO初始化 
#define cmt_rxant_setpin()      SET_GPIO_OUT(CMT_RX_ANT)//GPIO_Pin_Setting(CMT_RX_ANT, CMT_RX_ANT_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define cmt_txant_setpin()      SET_GPIO_OUT(CMT_TX_ANT)//GPIO_Pin_Setting(CMT_TX_ANT, CMT_TX_ANT_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)

#define cmt_spi3_csb_1()        SET_GPIO_H(CMT_CSB_GPIO)//GPIO_SetBits(CMT_CSB_GPIO,CMT_CSB_GPIO_PIN)
#define cmt_spi3_csb_0()        SET_GPIO_L(CMT_CSB_GPIO)//GPIO_ResetBits(CMT_CSB_GPIO,CMT_CSB_GPIO_PIN)

#define cmt_spi3_fcsb_1()       SET_GPIO_H(CMT_FCSB_GPIO)//GPIO_SetBits(CMT_FCSB_GPIO,CMT_FCSB_GPIO_PIN)
#define cmt_spi3_fcsb_0()       SET_GPIO_L(CMT_FCSB_GPIO)//GPIO_ResetBits(CMT_FCSB_GPIO,CMT_FCSB_GPIO_PIN)
    
#define cmt_spi3_scl_1()        SET_GPIO_H(CMT_SCL_GPIO)//GPIO_SetBits(CMT_SCL_GPIO,CMT_SCL_GPIO_PIN) 
#define cmt_spi3_scl_0()        SET_GPIO_L(CMT_SCL_GPIO)//GPIO_ResetBits(CMT_SCL_GPIO,CMT_SCL_GPIO_PIN)

#define cmt_spi3_sda_1()        SET_GPIO_H(CMT_SDA_GPIO)//GPIO_SetBits(CMT_SDA_GPIO,CMT_SDA_GPIO_PIN)
#define cmt_spi3_sda_0()        SET_GPIO_L(CMT_SDA_GPIO)//GPIO_ResetBits(CMT_SDA_GPIO,CMT_SDA_GPIO_PIN)

#define cmt_rx_ant_1()          SET_GPIO_H(CMT_RX_ANT)//GPIO_SetBits(CMT_RX_ANT,CMT_RX_ANT_PIN)
#define cmt_rx_ant_0()          SET_GPIO_L(CMT_RX_ANT)//GPIO_ResetBits(CMT_RX_ANT,CMT_RX_ANT_PIN)

#define cmt_tx_ant_1()          SET_GPIO_H(CMT_TX_ANT)//GPIO_SetBits(CMT_TX_ANT,CMT_TX_ANT_PIN)
#define cmt_tx_ant_0()          SET_GPIO_L(CMT_TX_ANT)//GPIO_ResetBits(CMT_TX_ANT,CMT_TX_ANT_PIN)

#define cmt_spi3_sda_read()     READ_GPIO_PIN(CMT_SDA_GPIO)//GPIO_ReadInputDataBit(CMT_SDA_GPIO,CMT_SDA_GPIO_PIN)

#define ANT_RX()  {cmt_rx_ant_0();cmt_tx_ant_1();}
#define ANT_TX()  {cmt_rx_ant_1();cmt_tx_ant_0();}

#define SET_GPIO_OUT(x)             GPIO_Pin_Setting(x, x##_PIN,  GPIO_MODE_OUT_PP_LOW_FAST)
#define SET_GPIO_IN(x)              GPIO_Pin_Setting(x, x##_PIN,  GPIO_MODE_IN_FL_IT) //有中断
#define SET_GPIO_IN_NOINT(x)        GPIO_Pin_Setting(x, x##_PIN,  GPIO_MODE_IN_FL_NO_IT)
#define SET_GPIO_OD(x)              GPIO_Pin_Setting(x, x##_PIN,  GPIO_MODE_OUT_OD_LOW_FAST)
//#define SET_GPIO_AIN(x)             GPIO_Pin_Setting(x, x##_PIN,  GPIO_Mode_AIN)
//#define SET_GPIO_AFOUT(x)           GPIO_Pin_Setting(x, x##_PIN,  GPIO_Mode_AF_PP)
//#define SET_GPIO_AFOD(x)            GPIO_Pin_Setting(x, x##_PIN,  GPIO_Mode_AF_OD)
#define SET_GPIO_H(x)               (x->ODR |= (uint8_t)x##_PIN) //GPIO_SetBits(x, x##_PIN)
#define SET_GPIO_L(x)               (x->ODR &= (uint8_t)~x##_PIN) //GPIO_ResetBits(x, x##_PIN)##表示宏结合 结合过后再展开
#define READ_GPIO_PIN(x)            (((x->IDR & x##_PIN)!=0) ?1 :0) //GPIO_ReadInputDataBit(x, x##_PIN)

void cmt_spi3_delay(void);
void cmt_spi3_delay_us(void);

void cmt_spi3_init(void);

void cmt_spi3_send(u8 data8);
u8 cmt_spi3_recv(void);

void cmt_spi3_write(u8 addr, u8 dat);
void cmt_spi3_read(u8 addr, u8* p_dat);

void cmt_spi3_write_fifo(const u8* p_buf, u16 len);//
void cmt_spi3_read_fifo(u8* p_buf, u16 len);
void GPIO_Pin_Setting(GPIO_TypeDef *gpio, GPIO_Pin_TypeDef nPin,GPIO_Mode_TypeDef mode);