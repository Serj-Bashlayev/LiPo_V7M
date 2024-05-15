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

// ����������� �������� ������� � BRIGHT_UHI �� BRIGHT_HI
// ��� (TEMP_OFF_DEF_SET + 10) - ����������
// �������� �� ��������� (�� ����������)
#define TEMP_OFF_DEF_SET 65

typedef enum
{
  MODE_PW_OFF = 0,  // ��������
  MODE_1,           // ������� ������
  MODE_2,           // ������� ����������
  MODE_3,           // ������� ������� ������
  MODE_DO_PW_OFF    // ����������
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
