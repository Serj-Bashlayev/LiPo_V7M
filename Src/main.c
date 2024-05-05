//===========================================================================
// FILE:        main.c
// AUTHOR:      AVSel
// VERSION:     7.0m
// DATE:        05/03/2014
// LAST MODIFICATION : Serj Balabay (17.04.2020)
//===========================================================================

#include "hard.h"
#include "misc.h"

#define DBL_PRESS_TIME 400
#define LONG_PRESS_TIME 800

typedef enum {
  KEY_UNPRESS = 0,  /*  __                        */
  KEY_WAIT_UNPRESS, /*                            */
  KEY_PRESS1,       /*  __/                       */
  KEY_LONGPRESS,    /*  __/-----                  */
  KEY_UNPRESS1,     /*  __/--\_                   */
  KEY_PRESS2,       /*  __/--\__/                 */
  KEY_DBL_LONGPRESS,/*  __/--\__/-----            */
  KEY_UNPRESS2,     /*  __/--\__/--\_             */
  KEY_PRESS3,       /*  __/--\__/--\__/           */
  KEY_UNPRESS3,     /*  __/--\__/--\__/--\_       */
  KEY_PRESS4        /*  __/--\__/--\__/--\__/     */
} KEY_STATE_TD;

KEY_STATE_TD KeyState;
STATE_TD ModeOnOff;     // режим
BRIGHT_TD Line1_Bright,Line2_Bright,Line3_Bright; // яркость режима
BRIGHT_TD Bright;       // текущая яркость
unsigned char BlockPwOn;// включение фонарика заблокировано

// режим маячка
//  период мигания 10, 4 или 1 сек
//  время вспышки 0.1 сек
void BlinkMode(void)
{
  unsigned char i,
                j;
  LedOff();
  Delay(500);
  LedOnFast(Bright);
  KEY_PRESSED_ISR = 0;  // сброс флага нажатия кнопки
  Delay(200);
  j = 40;
  if (ModeOnOff == STATE_SHORT_ON)  {
    j = 16;
  }
  else if (ModeOnOff == STATE_DBLCLK_ON) {
    j = 4;
  }
  while (1)
  {
    LedOff();
    for (i = 0; i < j; i++) {
      Delay(250);
      ///if (!KEY_NOT_PRESSED())
      if (KEY_PRESSED_ISR)
        return;
      BatTest();
      if (ModeOnOff == STATE_DO_PW_OFF)
        return;
    }
    LedOnFast(Bright);
    Delay(100);
  }
}

// короткое нажатие
// PW_OFF: включение в режиме STATE_SHORT_ON
// PW_ON:  выключение
void ShortPress(void)
{
  if (BlockPwOn)
    ModeOnOff = STATE_DO_PW_OFF;
  if (ModeOnOff == STATE_DO_PW_OFF)
    return;
  KeyState = KEY_WAIT_UNPRESS;
  if (ModeOnOff == STATE_PW_OFF) {
    ModeOnOff = STATE_SHORT_ON;
    Bright = Line1_Bright;
    LedOnSlow(Bright);
  }
  else {
    ModeOnOff = STATE_DO_PW_OFF;
  }
}

// нажатие с удержанием 0.8 сек
// PW_OFF: включение в режиме STATE_LONG_ON
// PW_ON:  увеличение яркости
void LongPress(void)
{
  if (BlockPwOn)
    ModeOnOff = STATE_DO_PW_OFF;
  if (ModeOnOff == STATE_DO_PW_OFF)
    return;
  if (ModeOnOff == STATE_PW_OFF) {
    KeyState = KEY_WAIT_UNPRESS;
    ModeOnOff = STATE_LONG_ON;
    Bright = Line2_Bright;
    LedOnSlow(Bright);
  }
  else {
    KeyState = KEY_LONGPRESS;
    if (Bright) {
      Bright--;
      LedOnSlow(Bright);
    }
  }
  ClearTimer();
}

// двойное нажатиее
// PW_OFF: включение в режиме STATE_DBLCLK_ON
// PW_ON:  уменьшение яркости
void DblPress(void)
{
  if (BlockPwOn)
    ModeOnOff = STATE_DO_PW_OFF;
  if (ModeOnOff == STATE_DO_PW_OFF)
    return;
  KeyState = KEY_WAIT_UNPRESS;
  if (ModeOnOff == STATE_PW_OFF) {
    ModeOnOff = STATE_DBLCLK_ON;
    Bright = Line3_Bright;
    LedOnSlow(Bright);
  }
  else {
    if (Bright < BRIGHT_ULOW2) {
      Bright++;
      LedOnSlow(Bright);
    }
  }
}

// кратковременное нажатие + нажатие с удержанием
// PW_OFF: индикация напряжения аккумулятора
// PW_ON:  уменьшение яркости с повтором при удержании
void DblLongPress(void)
{
  if (BlockPwOn)
    ModeOnOff = STATE_DO_PW_OFF;
  if (ModeOnOff == STATE_DO_PW_OFF)
    return;
  if (ModeOnOff == STATE_PW_OFF) {
    KeyState = KEY_WAIT_UNPRESS;
    OutBattaryVoltage();
    ClearTimer();
    while (KEY_PRESSED())
    {
      if (GetTimer() > 6000) {
        CalibrationTemp();
      }
    }
    ModeOnOff = STATE_DO_PW_OFF;
  }
  else {
    KeyState = KEY_DBL_LONGPRESS;
    if (Bright < BRIGHT_ULOW2) {
      Bright++;
      LedOnSlow(Bright);
    }
  }
  ClearTimer();
}

// 3 нажатия
// PW_OFF: включение в режиме STATE_DBLCLK_ON
// PW_ON:  режим маячка
void Press3(void)
{
  if (BlockPwOn)
    ModeOnOff = STATE_DO_PW_OFF;
  if (ModeOnOff == STATE_DO_PW_OFF)
    return;
  KeyState = KEY_WAIT_UNPRESS;
  if (ModeOnOff == STATE_PW_OFF) {
    ModeOnOff = STATE_DBLCLK_ON;
    Bright = Line3_Bright;
    LedOnSlow(Bright);
  }
  else {
    BlinkMode();
    LedOnSlow(Bright);
  }
}

// 4 нажатия
// PW_OFF: блокировка/разблокировка включения фонарика
// PW_ON:  сохранение текущей яркости в EEPROM
void Press4(void)
{
  KeyState = KEY_WAIT_UNPRESS;
  switch (ModeOnOff) {
  case STATE_PW_OFF:
    // блокировка/разблокировка включения фонарика
    if (BlockPwOn) {
      BlockPwOn = 0;
      LedSysBlink();
    }
    else {
      BlockPwOn = 1;
    }
    LedSysBlink();
    ModeOnOff = STATE_DO_PW_OFF;
    return;
  // сохранение текущей яркости в EEPROM
  case STATE_SHORT_ON:
    Line1_Bright = Bright;
    WriteCfgByte(CFG_ADRESS_MODE1, Bright);
    break;
  case STATE_LONG_ON:
    Line2_Bright = Bright;
    WriteCfgByte(CFG_ADRESS_MODE2, Bright);
    break;
  case STATE_DBLCLK_ON:
    Line3_Bright = Bright;
    WriteCfgByte(CFG_ADRESS_MODE3, Bright);
    break;
  }
  // LED double blink
  LedOff();
  Delay(250);
  LedOnFast(Bright);
  Delay(250);
  LedOff();
  Delay(250);
  LedOnFast(Bright);
}

void main(void)
{
  unsigned char Key;

  LoadCfg();
  InitHard();
  Delay(20);
  while (1)
  {
    Key = KEY_PRESSED() ? 1 : 0;

    switch (KeyState) {
    case KEY_WAIT_UNPRESS:
      if (!KEY_PRESSED())
        KeyState = KEY_UNPRESS;
      break;
    case KEY_UNPRESS:
      if (Key) {
        KeyState = KEY_PRESS1;
        ClearTimer();
      }
      else {
        if (ModeOnOff == STATE_PW_OFF)
          ModeOnOff = STATE_DO_PW_OFF;
      }
      break;
    case KEY_PRESS1:
      if (Key) {
        if (GetTimer() > LONG_PRESS_TIME) {
          LongPress();
        }
      }
      else {
        KeyState = KEY_UNPRESS1;
      }
      break;
    case KEY_LONGPRESS:
      if (Key) {
        if (GetTimer() > LONG_PRESS_TIME)
          LongPress();
      }
      else {
        KeyState = KEY_UNPRESS;
      }
      break;
    case KEY_UNPRESS1:
      if (Key) {
        KeyState = KEY_PRESS2;
        ClearTimer();
      }
      else {
        if (GetTimer() > DBL_PRESS_TIME)
          // короткое нажатие
          ShortPress();
      }
      break;
    case KEY_PRESS2:
      if (Key) {
        if (GetTimer() > LONG_PRESS_TIME) {
          // кратковременное нажатие + нажатие с удержанием
          DblLongPress();
        }
      }
      else {
        KeyState = KEY_UNPRESS2;
      }
      break;
    case KEY_DBL_LONGPRESS:
      if (Key) {
        if (GetTimer() > LONG_PRESS_TIME)
          DblLongPress();
      }
      else {
        KeyState = KEY_UNPRESS;
      }
      break;
    case KEY_UNPRESS2:
      if (Key) {
        KeyState = KEY_PRESS3;
        ClearTimer();
      }
      else {
        if (GetTimer() > DBL_PRESS_TIME)
          // двойное нажатие
          DblPress();
      }
      break;
    case KEY_PRESS3:
      if (Key) {
        if (GetTimer() > LONG_PRESS_TIME)
          KeyState = KEY_WAIT_UNPRESS;
      }
      else {
        KeyState = KEY_UNPRESS3;
      }
      break;
    case KEY_UNPRESS3:
      if (Key) {
        KeyState = KEY_PRESS4;
        ClearTimer();
      }
      else {
        if (GetTimer() > DBL_PRESS_TIME)
          // тройное нажатие
          Press3();
      }
      break;
    case KEY_PRESS4:
      if (Key) {
        if (GetTimer() > LONG_PRESS_TIME)
          KeyState = KEY_WAIT_UNPRESS;
      }
      else {
        Press4();
      }
      break;
    }
    Wait30ms();
    if (ModeOnOff == STATE_DO_PW_OFF) {
      LedOff();
      ModeOnOff = STATE_PW_OFF;
      KeyState = KEY_UNPRESS;
      while (KEY_PRESSED())
        Wait30ms();
      Wait30ms();
      PowerOff();
      Wait30ms();
    }
    else {
      BatTest();
      TempTest();
    }
  }
}

