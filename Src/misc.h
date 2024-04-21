//===========================================================================
// FILE:      misc.h
// AUTHOR:    AVSel
// VERSION:   7.0m
// DATE:      05/03/2014
// LAST MODIFICATION : Serj Balabay (17.04.2020)
//===========================================================================

#ifndef _MISC_H
#define _MISC_H

#define CFG_ADRESS_TEMP  1
#define CFG_ADRESS_MODE1 2
#define CFG_ADRESS_MODE2 3
#define CFG_ADRESS_MODE3 4

#define TEMP_DEF_SET 358 /* 75C */

typedef enum
{
  STATE_PW_OFF = 0, // выключен
  STATE_SHORT_ON,   // включен коротким нажатием
  STATE_LONG_ON,    // включен нажатием с удержанием 0.8 сек
  STATE_DBLCLK_ON,  // включен двойным нажатием
  STATE_DO_PW_OFF   // выключение
} STATE_TD;

extern STATE_TD ModeOnOff;

unsigned char ReadCfgByte(unsigned char adress, unsigned char *byte);
void WriteCfgByte(unsigned char adress, unsigned char byte);
void LoadCfg(void);

void OutBattaryVoltage(void);
void CalibrationTemp(void);
void BatTest(void);
void TempTest(void);
unsigned short filter(unsigned short x, char init);

#endif
