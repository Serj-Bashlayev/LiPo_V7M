//===========================================================================
// FILE:      misc.c
// AUTHOR:    AVSel
// VERSION:   7.5m
// DATE:      05/03/2014
// LAST MODIFICATION : Serj Balabay (16.05.2024)
//===========================================================================

#include "hard.h"
#include "misc.h"

extern BRIGHT_TD Mode1_Bright,Mode2_Bright,Mode3_Bright;;
extern BRIGHT_TD Bright;

unsigned char TempCount;
unsigned char BatCount;
signed char   TempOff; // “емпература снижени€ €ркости с BRIGHT_UHI до BRIGHT_HI. (TEMP_OFF_DEF_SET + 10) - отключение

unsigned char ReadCfgByte(unsigned char adress, unsigned char *byte)
{
  unsigned char c1,
                c2;
  *byte = ReadEEPROMByte(adress);
  c1 = ReadEEPROMByte(adress + 0x10) ^ 0xAA;
  c2 = ReadEEPROMByte(adress + 0x20) ^ 0x55;
  if (*byte == c1) {
    if (*byte != c2) {
      WriteEEPROMByte(adress + 0x20, *byte ^ 0x55);
    }
  }
  else {
    if (*byte == c2) {
      WriteEEPROMByte(adress + 0x10, *byte ^ 0xAA);
    }
    else {
      if (c1 == c2) {
        *byte = c1;
        WriteEEPROMByte(adress, c1);
      }
      else {
        return 1; //Error
      }
    }
  }
  return 0;
}

void WriteCfgByte(unsigned char adress, unsigned char byte)
{
  WriteEEPROMByte(adress, byte);
  WriteEEPROMByte(adress + 0x10, byte ^ 0xAA);
  WriteEEPROMByte(adress + 0x20, byte ^ 0x55);
}

void LoadCfg(void)
{
  if (ReadCfgByte(CFG_ADRESS_MODE1, (unsigned char*)&Mode1_Bright))
    Mode1_Bright = BRIGHT_MID;
  if (ReadCfgByte(CFG_ADRESS_MODE2, (unsigned char*)&Mode2_Bright))
    Mode2_Bright = BRIGHT_ULOW2;
  if (ReadCfgByte(CFG_ADRESS_MODE3, (unsigned char*)&Mode3_Bright))
    Mode3_Bright = BRIGHT_UHI;
  if (ReadCfgByte(CFG_ADRESS_TEMP, (unsigned char*)&TempOff))
    TempOff = TEMP_OFF_DEF_SET;
}

// ѕороги отображени€ напр€жени€ аккумул€тора (1..5 вспышек)
__flash unsigned short BatVolt[] = {
  BAT_CALC(3.40),
  BAT_CALC(3.65),
  BAT_CALC(3.85),
  BAT_CALC(4.05)
};

void OutBattaryVoltage(void)
{
  unsigned char  i;
  unsigned short Bat;
  LedOff();
  Delay(250);
  Bat = GetBat();
  Delay(250);
  Bat += GetBat();
  Bat = Bat >> 1;
  for (i = 1; i < 5; i++) {
    if (Bat < BatVolt[i - 1])
      break;
  }
  while (i--)
  {
    LedSysBlink();
  }
}

void CalibrationTemp(void)
{
  TempOff = TEMP_OFF_DEF_SET;
  WriteCfgByte(CFG_ADRESS_TEMP, TempOff);
  LedOnFast(BRIGHT_UHI);
  while (KEY_PRESSED());
  Delay(500);
  while (!KEY_PRESSED());
  LedOff();
  Delay(200);
  while (KEY_PRESSED())
  {
    if (GetTimer() > 2000) {
      TempOff = GetTemp();
      LedSysBlink();
      break;
    }
  }
  WriteCfgByte(CFG_ADRESS_TEMP, TempOff);
  LedSysBlink();
}

// снижение €ркости при разр€де аккумул€тора
__flash unsigned short VModeDown[] = {
  BAT_CALC(2.75),
  BAT_CALC(2.85),
  BAT_CALC(3.00),
  BAT_CALC(3.12),
  BAT_CALC(3.18),
  BAT_CALC(3.20)
};

// если MODE_2 (включен удержанием) - 
// "экономный" режим снижени€ €ркости при разр€де аккумул€тора
__flash unsigned short VModeDownE[] = {
  BAT_CALC(2.75),
  BAT_CALC(3.00),
  BAT_CALC(3.15),
  BAT_CALC(3.30),
  BAT_CALC(3.45),
  BAT_CALC(3.50)
};

/*
  ”меньшение €ркости в зависимости от напр€жени€ аккумул€тора
*/
void BatTest(void)
{
  unsigned short vdown;

  if (Mode == MODE_2) {
    vdown = VModeDownE[Bright];
  }
  else {
    vdown = VModeDown[Bright];
  }

  if (GetBat() < vdown) {
    BatCount++;
    if (BatCount > 10) {
      if (Bright == BRIGHT_ULOW2) {
        Mode = MODE_DO_PW_OFF;
      }
      else {
        Bright--;
        LedOnSlow(Bright);
      }
      BatCount = 0;
    }
  }
  else {
    BatCount = 0;
  }
}

/*
  1. при температуре > (заданна€ + 10) - ќтключение фонар€
  2. при температуре > заданна€ - снижение €ркости с 2000мј до 600мј
  (10 измереий подр€д)
*/
void TempTest(void)
{
  signed char temp;
  temp = GetTemp();
  TempCount++;
  if (temp > TempOff + 10) {
    if (TempCount > 10) {
      Mode = MODE_DO_PW_OFF;
    }
    return;
  }
  if (temp > TempOff) {
    if ((TempCount > 10) && (Bright == BRIGHT_UHI)) {
      Bright = BRIGHT_HI;
      LedOnSlow(Bright);
    }
    return;
  }
  TempCount = 0;
}


#define k   4
#define Nb  1 // Nb = 1,2,3...(2^k)
              //    1   - сама€ сильна€ фильтраци€
              //    2^k - отсутствие фильтрации
unsigned short filter(unsigned short x, char init)
{
  static unsigned short y = 0;
  static unsigned short z = 0;

  if (init) {
    z = (x << k) / Nb;
    y = x;
    return x;
  }
  else {
    z += (x - y);
  }
  return y = (Nb * z) >> k;
}

