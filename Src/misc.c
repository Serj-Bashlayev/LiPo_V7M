//===========================================================================
// FILE:      misc.c
// AUTHOR:    AVSel
// VERSION:   7.0m
// DATE:      05/03/2014
// LAST MODIFICATION : Serj Balabay (17.04.2020)
//===========================================================================

#include "hard.h"
#include "misc.h"

//extern unsigned char Line1Mode,Line2Mode,Line3Mode;
//extern unsigned char Mode;
extern BRIGHT_TD Line1_Bright,Line2_Bright,Line3_Bright;;
extern BRIGHT_TD Bright;

unsigned char TempCount;
unsigned char BatCount;
UniShort      TempOff;

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
  if (ReadCfgByte(CFG_ADRESS_MODE1, (unsigned char*)&Line1_Bright))
    Line1_Bright = BRIGHT_MID;
  if (ReadCfgByte(CFG_ADRESS_MODE2, (unsigned char*)&Line2_Bright))
    Line2_Bright = BRIGHT_ULOW2;
  if (ReadCfgByte(CFG_ADRESS_MODE3, (unsigned char*)&Line3_Bright))
    Line3_Bright = BRIGHT_UHI;
  if (ReadCfgByte(CFG_ADRESS_TEMP, &(TempOff.c[0]))) {
    TempOff.s = TEMP_DEF_SET;
  }
  else {
    TempOff.s += 200;
  }
}

__flash unsigned short BatVolt[] = {
  BAT_CALC(3.40),
  BAT_CALC(3.65),
  BAT_CALC(3.85),
  BAT_CALC(4.05) };

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
  TempOff.s = TEMP_DEF_SET;
  WriteCfgByte(CFG_ADRESS_TEMP, (unsigned char)(TempOff.s - 200));
  LedOnFast(BRIGHT_UHI);
  while (!KEY_NOT_PRESSED());
  Delay(500);
  while (KEY_NOT_PRESSED());
  LedOff();
  Delay(200);
  while (!KEY_NOT_PRESSED())
  {
    if (GetTimer() > 2000) {
      TempOff.s = GetTemp();
      LedSysBlink();
      break;
    }
  }
  WriteCfgByte(CFG_ADRESS_TEMP, (unsigned char)(TempOff.s - 200));
  LedSysBlink();
}

__flash unsigned short VModeDown[] = {
  BAT_CALC(3.20 - BAT_SHOTTKY),
  BAT_CALC(3.18 - BAT_SHOTTKY),
  BAT_CALC(3.12 - BAT_SHOTTKY),
  BAT_CALC(3.00 - BAT_SHOTTKY),
  BAT_CALC(2.85 - BAT_SHOTTKY),
  BAT_CALC(2.75 - BAT_SHOTTKY) };

// если включен с удержанием 0.8 сек
__flash unsigned short VModeDownE[] = {
  BAT_CALC(3.50 - BAT_SHOTTKY),
  BAT_CALC(3.45 - BAT_SHOTTKY),
  BAT_CALC(3.30 - BAT_SHOTTKY),
  BAT_CALC(3.15 - BAT_SHOTTKY),
  BAT_CALC(3.00 - BAT_SHOTTKY),
  BAT_CALC(2.75 - BAT_SHOTTKY) };

void BatTest(void)
{
  unsigned short vdown;

  if (ModeOnOff == STATE_LONG_ON) {
    vdown = VModeDownE[Bright];
  }
  else {
    vdown = VModeDown[Bright];
  }

  if (GetBat() < vdown) {
    BatCount++;
    if (BatCount > 10) {
      if (Bright >= BRIGHT_ULOW2) {
        ModeOnOff = STATE_DO_PW_OFF;
      }
      else {
        Bright++;
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
  1. Отключение фонаря при температуре > (заданная + 10)
  2. Снижение яркости с 2000мА до 600мА при температуре > заданная
  (10 измереий подряд)
*/
void TempTest(void)
{
  unsigned short temp;
  temp = GetTemp();
  TempCount++;
  if (temp > TempOff.s + 10) {
    if (TempCount > 10) {
      ModeOnOff = STATE_DO_PW_OFF;
    }
    return;
  }
  if (temp > TempOff.s) {
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
              //    1   - самая сильная фильтрация
              //    2^k - отсутсвие фильртрации
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

