#ifndef __IR_H
#define __IR_H
#include "sys.h"
#include "protocol.h"

extern volatile u8 	 uc_vgSendIRFlag;

void IRSendStudy(ST_RFIRStudy *stp_argIRStudy);
void DisCode_Ir(const u8 *p,u8 len);
void SendIRFormlib(void);

#endif
