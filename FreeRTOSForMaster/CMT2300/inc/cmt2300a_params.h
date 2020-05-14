#ifndef __CMT2300A_PARAMS_H
#include "cmt2300_defs.h"
//#include "typedefs.h"
#include "sys.h"

const u8 CMTBank[12] = {
					0x00,
					0x66,
					0xEC, 
					0x1D,
					0xF0,
					0x80,
					0x14,
					0x08,
					0x91,
					0x02,
					0x02,
					0xD0
				   };

/*******System Configuration********/
const u8 SystemBank[12] = {
				    0xAE,   // rx current regulation
				    0xE0,   // Duty-Cycle Control Register
				    0x35,   // Duty-Cycle Control Register
				    0x00,   //Sleep Timer 
				    0x00,   //Sleep Timer 
				    0xF4,   //RX Sleep T1 Timer 
				    0x10,   //RX Sleep T1 Timer 
				    0xE2,   //RX Sleep T2 Timer 
				    0x42,   //RX Sleep T2 Timer 
				    0x20,   //SLP Register
				    0x00,   // PJD Register
				    0x81	  // PJD Register			
				    };

/*******Frequency Configuration********/
/*************433.92Mhz***************/
const u8 FrequencyBank[8] = {
					0x42,
					0x71,
					0xCE,
					0x1C,
					0x42,
					0x5B,
					0x1C,
					0x1C		    
					     };

/*******Data_Rate_Configuration********/
/*************40kpbs***************/
const u8 DataRateBank[24] = {
					0xD3,
					0x64,
					0xA0,
					0x99,
					0x00,
					0x00,
					0x00,
					0x00,
					0x00,
					0x00,
					0x00,
					0x29,   //CDR Control Register
					0xC0,   //CDR Control Register
					0x28,   //CDR Control Register
					0x0A,   //CDR Control Register
					0x5B,   
					0x07,  
					0x00,
					0x50,
					0x2D,
					0x00,
					0x01,
					0x05,
					0x05
						};     

/*******Basebank Configuration********/
const u8 BasebandBank[29] = {
					0x10,   //Data_mode Register
					0x08,   //TX_preamble_size Rigister
					0x00,   //TX_preamble_size Rigister
					0xAA,   //TX_preamble_value Rigister (0xAA)
					0x02,   //Sysc_Word_Configuration Register
					0x00,   //Sysc_Word_Value 
					0x00,   //Sysc_Word_Value 
					0x00,   //Sysc_Word_Value 
					0x00,   //Sysc_Word_Value 
					0x00,   //Sysc_Word_Value 
					0x00,   //Sysc_Word_Value 
					0xD4,   //Sysc_Word_Value 
					0x2D,   //Sysc_Word_Value 
					0x00,   //Payload_Configuration
					0x1F,   //Payload_length
					0x00,   //Node_ID_Configuration
					0x00,   //Node_ID_Value 
					0x00,   //Node_ID_Value 
					0x00,   //Node_ID_Value 
					0x00,   //Node_ID_Value 
					0x00,   //FEC_Configuration & CRC_Configuration
					0x00,   //CRC_Seed
					0x00,   //CRC_Seed
					0x60,   //CRC_Bit_Order
					0xFF,   //Whiten_seed
					0x00,   //Tx_Prefix_type value
					0x00,   //Tx_packet_number register
					0x1F,   //Tx_packet_gap register
					0x10	
						};	

/*******Tx Configuration********/
const u8 TXBank[11] = {
					0x55,
					0x26,
					0x03,
					0x00,
					0x03,
					0x70,
					0x00,
					0x8A,  //20dBm
					0x18,
					0x3F,
					0x7F															
				};

#endif

