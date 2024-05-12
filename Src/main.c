//===========================================================================
// FILE:        main.c
// DESCRIPTION:
//
// AUTHOR:      AVSel
// VERSION:     7.4m
// DATE:        03.05.2014
//
// LAST MODIFICATION : Serj Balabay (12.05.2024)
// HISTORY:
// 7.0m - (03.05.2014) »сходный файл от AVSel с форума фонарЄвки
// 7.1m - (17.04.2020) ѕереработанна€ программа
// 7.2m - (06.05.2024) јнализ состо€ни€ нажатий кнопок вынесен в отделный файл.
//                     »ндикаци€ уровн€ аккумул€тора перенесена на тройной клик.
//                     ѕереименование некоторых переменных.
// 7.3m - (06.05.2024) ¬ключение коротким нажатием на той-же €ркости, на которой выключили.
//                     ѕосле включени€ длительным удержанием ждЄм отпускани€ кнопки (не даЄм увеличивает €ркость).
// 7.4m - (12.05.2024) ѕосле выхода из режима ма€чка ждЄм отпускани€ кнопки.
//
//===========================================================================

#include "hard.h"
#include "misc.h"
#include "key.h"

MODE_TD       Mode;      // режим
BRIGHT_TD     Mode1_Bright, Mode2_Bright, Mode3_Bright; // €ркость режима
BRIGHT_TD     Bright;    // текуща€ €ркость
unsigned char PowerOnBlocked; // включение фонарика заблокировано


// режим ма€чка
// период мигани€ 10, 4 или 1 сек
// врем€ вспышки 0.1 сек
void BlinkMode(void)
{
  unsigned char i,
                j;
  LedOff();
  Delay(500);
  LedOnFast(Bright);
  KEY_PRESSED_ISR = 0;  // сброс флага нажати€ кнопки
  Delay(200);

  j = 40;
  if (Mode == MODE_1)
    j = 16;
  else if (Mode == MODE_3)
    j = 4;

  while (1)
  {
    LedOff();
    for (i = 0; i < j; i++) {
      Delay(250);
      if (KEY_PRESSED_ISR)
        return;

      BatTest();
      if (Mode == MODE_DO_PW_OFF)
        return;
    }
    LedOnFast(Bright);
    Delay(100);
  }
}

// короткое нажатие
// PW_OFF: включение в режиме 1
// PW_ON:  выключение
void Click(void)
{
  if (PowerOnBlocked)
    Mode = MODE_DO_PW_OFF;
  if (Mode == MODE_DO_PW_OFF)
    return;

  if (Mode == MODE_PW_OFF) {
    Mode = MODE_1;
    Bright = Mode1_Bright;
    LedOnSlow(Bright);
  }
  else {
    if (Mode == MODE_1) {
      // ¬ первом режиме:
      // включение коротким нажатием на той-же €ркости, на которой выключили
      Mode1_Bright = Bright;
    }
    Mode = MODE_DO_PW_OFF;
  }
}

// ”держание 0.8 сек
// PW_OFF: включение в режиме 2
// PW_ON:  увеличение €ркости
void LongPress(void)
{
  if (PowerOnBlocked)
    Mode = MODE_DO_PW_OFF;
  if (Mode == MODE_DO_PW_OFF)
    return;

  if (Mode == MODE_PW_OFF) {
    Key_Set_RELEASE();
    Mode = MODE_2;
    Bright = Mode2_Bright;
    LedOnSlow(Bright);
  }
  else {
    if (Bright < BRIGHT_UHI) {
      Bright++;
      LedOnSlow(Bright);
    }
  }
}

// двойное нажатие
// PW_OFF: включение в режиме 3
// PW_ON:  уменьшение €ркости
void Click_2(void)
{
  if (PowerOnBlocked)
    Mode = MODE_DO_PW_OFF;
  if (Mode == MODE_DO_PW_OFF)
    return;

  if (Mode == MODE_PW_OFF) {
    Mode = MODE_3;
    Bright = Mode3_Bright;
    LedOnSlow(Bright);
  }
  else {
    if (Bright > BRIGHT_ULOW2) {
      Bright--;
      LedOnSlow(Bright);
    }
  }
}

// 3 клика
// PW_OFF: индикаци€ напр€жени€ аккумул€тора
// PW_ON:  режим ма€чка
void Click_3(void)
{
  if (PowerOnBlocked)
    Mode = MODE_DO_PW_OFF;
  if (Mode == MODE_DO_PW_OFF)
    return;

  if (Mode == MODE_PW_OFF) {
    OutBattaryVoltage();
  }
  else {
    BlinkMode();
    Key_Set_RELEASE();
    LedOnSlow(Bright);
  }
}

// 4 нажати€
// PW_OFF: блокировка/разблокировка включени€ фонарика
// PW_ON:  сохранение текущей €ркости в EEPROM
void Click_4(void)
{
  switch (Mode) {
  case MODE_PW_OFF:
    // блокировка/разблокировка включени€ фонарика
    if (PowerOnBlocked) {
      PowerOnBlocked = 0;
      LedSysBlink();
    }
    else {
      PowerOnBlocked = 1;
    }
    LedSysBlink();
    Mode = MODE_DO_PW_OFF;
    return;
    // сохранение текущей €ркости в EEPROM
  case MODE_1:
    Mode1_Bright = Bright;
    WriteCfgByte(CFG_ADRESS_MODE1, Bright);
    break;
  case MODE_2:
    Mode2_Bright = Bright;
    WriteCfgByte(CFG_ADRESS_MODE2, Bright);
    break;
  case MODE_3:
    Mode3_Bright = Bright;
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

// клик + нажатие с удержанием
// PW_OFF: удержание более 6 сек - старт калибровки температуры
// PW_ON:  уменьшение €ркости с повтором при удержании
void DblLongPress(void)
{
  if (PowerOnBlocked)
    Mode = MODE_DO_PW_OFF;
  if (Mode == MODE_DO_PW_OFF)
    return;

  if (Mode == MODE_PW_OFF) {
    ClearTimer();
    while (KEY_PRESSED())
    {
      if (GetTimer() > 6000) {
        CalibrationTemp();
      }
    }
    Mode = MODE_DO_PW_OFF;
  }
  else {
    if (Bright > BRIGHT_ULOW2) {
      Bright--;
      LedOnSlow(Bright);
    }
  }

  ClearTimer();
}


void main(void)
{
  KEY_STATE_TD KeyState;

  LoadCfg();
  InitHard();
  Delay(20);
  while (1)
  {
    KeyState = Key_State();

    switch (KeyState.State) {
    case S_CLICK:
      switch (KeyState.Counter) {
      case 1:
        // короткое нажатие
        Click();
        break;
      case 2:
        // двойное нажатиее
        Click_2();
        break;
      case 3:
        // 3 нажати€
        Click_3();
        break;
      case 4:
        // 4 нажати€
        Click_4();
        break;
      }
      break;
    case S_LONGPRESS_IMP:
      switch (KeyState.Counter) {
      case 1:
        // длинное нажатие
        LongPress();
        break;
      case 2:
        // короткое + длинное нажатие
        DblLongPress();
        break;
      }
    }

    Wait30ms();
    if (Mode == MODE_DO_PW_OFF) {
      // ¬ыключение
      LedOff();
      Mode = MODE_PW_OFF;
      while (KEY_PRESSED()) {
        Wait30ms();
      }
      Key_Reset_SM();

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

