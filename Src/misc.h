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

// температура снижения яркости с BRIGHT_UHI до BRIGHT_HI
// при (TEMP_OFF_DEF_SET + 10) - отключение
// значение по умолчанию (до калибровки)
#define TEMP_OFF_DEF_SET 65

typedef enum
{
  MODE_PW_OFF = 0,  // выключен
  MODE_1,           // включен кликом
  MODE_2,           // включен удержанием
  MODE_3,           // включен двойным кликом
  MODE_DO_PW_OFF    // выключение
} MODE_TD;

extern MODE_TD Mode;

unsigned char ReadCfgByte(unsigned char adress, unsigned char *byte);
void WriteCfgByte(unsigned char adress, unsigned char byte);
void LoadCfg(void);

void OutBattaryVoltage(void);
void CalibrationTemp(void);
void BatTest(void);
void TempTest(void);
unsigned short filter(unsigned short x, char init);

#endif
