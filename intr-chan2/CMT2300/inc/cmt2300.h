#ifndef __CMT2300_H
#define __CMT2300_H

//#include "typedefs.h"

#include "delay.h"

#define BOOL bool

void Cmt2300_SoftReset(void);
u8 Cmt2300_GetChipStatus(void);
BOOL Cmt2300_WaitChipStatus(u8 nStatus);
BOOL Cmt2300_GoSleep(void);
BOOL Cmt2300_GoStby(void);
BOOL Cmt2300_GoTFS(void);
BOOL Cmt2300_GoRFS(void);
BOOL Cmt2300_GoTx(void);
BOOL Cmt2300_GoRx(void);
BOOL Cmt2300_GoSwitch(void);
BOOL CMT2300_ForceRX(void);
BOOL CMT2300_ForceTX(void);

/* ************************************************************************
*  The following are for chip interrupts, GPIO, FIFO operations.
*  ************************************************************************ */
void Cmt2300_ConfigGpio(u8 nGpioSel);
void Cmt2300_ConfigInterrupt(u8 nInt1Sel, u8 nInt2Sel);
void Cmt2300_SetInterruptPolar(BOOL bActiveHigh);
void Cmt2300_SetFifoThreshold(u8 nFifoThreshold);
void Cmt2300_EnableAntennaSwitch(u8 nMode);
void Cmt2300_EnableInterrupt(u8 nEnable);
void Cmt2300_EnableRxFifoAutoClear(BOOL bEnable);
void Cmt2300_EnableFifoMerge(BOOL bEnable);
void Cmt2300_EnableReadFifo(void);
void Cmt2300_EnableWriteFifo(void);
void Cmt2300_RestoreFifo(void);
u8 Cmt2300_ClearFifo(void);
u8 CMT2300A_ClearRxFifo(void);
u8 CMT2300A_ClearTxFifo(void);
void CMT2300A_RestoreFifo(void);
u8 Cmt2300_ClearInterruptFlags(void);

/* ************************************************************************
*  The following are for Tx DIN operations in direct mode.
*  ************************************************************************ */
void Cmt2300_ConfigTxDin(u8 nDinSel);
void Cmt2300_EnableTxDin(BOOL bEnable);
void Cmt2300_EnableTxDinInvert(BOOL bEnable);


/* ************************************************************************
*  The following are general operations.
*  ************************************************************************ */
BOOL Cmt2300_IsExist(void);
u8 Cmt2300_GetRssiCode(void);
int Cmt2300_GetRssiDBm(void);
void Cmt2300_SetFrequencyChannel(u8 nChann);
void Cmt2300_SetFrequencyStep(u8 nOffset);
void Cmt2300_SetPayloadLength(u16 nLength);
void Cmt2300_EnableLfosc(BOOL bEnable);
void Cmt2300_EnableLfoscOutput(BOOL bEnable);
void Cmt2300_SetSyncLength(u8 nLength);
void Cmt2300_SetSynWord(u32 nWord);
void Cmt2300_ReadSynWord(u32 * syn);
void Cmt2300_SetResendTime(u8 nLength);

/* ************************************************************************
*  The following are for chip initializes.
*  ************************************************************************ */
void Cmt2300_Init(void);
BOOL Cmt2300_ConfigRegBank(u8 base_addr, const u8 bank[], u8 len);
void RF_Init(void);

#endif
