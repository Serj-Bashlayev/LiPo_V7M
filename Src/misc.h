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
  STATE_PW_OFF = 0, // ��������
  STATE_SHORT_ON,   // ������� �������� ��������
  STATE_LONG_ON,    // ������� �������� � ���������� 0.8 ���
  STATE_DBLCLK_ON,  // ������� ������� ��������
  STATE_DO_PW_OFF   // ����������
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
