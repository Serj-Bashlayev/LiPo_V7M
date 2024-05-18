//===========================================================================
// FILE:        main.c
// DESCRIPTION:
//
// AUTHOR:      AVSel
// VERSION:     7.5m
// DATE:        03.05.2014
//
// LAST MODIFICATION : Serj Balabay (16.05.2024)
// HISTORY:
// 7.0m - (03.05.2014)
//    Исходный файл от AVSel с форума фонарёвки
//
// 7.1m - (17.04.2020)
//    Переработанная программа
//
// 7.2m - (06.05.2024)
//    Переработан и вынесен в отдельный файл анализ состояния нажатий кнопки.
//    Индикация уровня аккумулятора перенесена на тройной клик.
//
// 7.3m - (06.05.2024) 
//    В основном режиме (включение коротким нажатием) – фонарик включается на той же яркости, на которой был выключен.
//    После включения длительным удержанием (экономичный режим) - ждём отпускания кнопки (блокируется непроизвольное увеличение яркости).
//
// 7.4m - (12.05.2024)
//    После выхода из режима маячка ожидаем отпускания кнопки (блокируется непроизвольное увеличение яркости).
//
// 7.5m - (16.05.2024)
//    Для отладки добавлена возможность вывода напряжения аккумулятора и температуры по UART (pin PB0, 7874 bits/s).
//    Вывод в UART - тройной клик с удержанием. (UART включается определением символа UART_DEBUG в настройках компилятора)
//    - После измерений скорректирован BAT_COEF. Точность измерения напряжения +-25 мВ
//    - После измерений скорректирован T_OFFSET. Точность измерения температуры +-2°C
//    - Коррекция падения на диоде Шоттки перенесена в макрос BAT_CALC(v)
//    - GetTemp() возвращает температуру в Цельсиях.
//    - TempOff в EEPROM сохраняется в Цельсиях
//    - Пороговая температура снижения яркости уменьшена с 75°С до 65°С.
//===========================================================================

#include "hard.h"
#include "misc.h"
#include "key.h"
#include <stdio.h>

MODE_TD       Mode;      // режим
BRIGHT_TD     Mode1_Bright, Mode2_Bright, Mode3_Bright; // яркость режима
BRIGHT_TD     Bright;    // текущая яркость
unsigned char PowerOnBlocked; // включение фонарика заблокировано


// режим маячка
// период мигания 10, 4 или 1 сек
// время вспышки 0.1 сек
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
      // В первом режиме:
      // включение коротким нажатием на той же яркости, на которой выключили
      Mode1_Bright = Bright;
    }
    Mode = MODE_DO_PW_OFF;
  }
}

// Удержание 0.8 сек
// PW_OFF: включение в режиме 2
// PW_ON:  увеличение яркости
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
// PW_ON:  уменьшение яркости
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
// PW_OFF: индикация напряжения аккумулятора
// PW_ON:  режим маячка
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

// 4 нажатия
// PW_OFF: блокировка/разблокировка включения фонарика
// PW_ON:  сохранение текущей яркости в EEPROM
void Click_4(void)
{
  switch (Mode) {
  case MODE_PW_OFF:
    // блокировка/разблокировка включения фонарика
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
    // сохранение текущей яркости в EEPROM
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
// PW_ON:  уменьшение яркости с повтором при удержании
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
#ifdef UART_DEBUG
  unsigned short bat;
  unsigned short bat_mV;
  unsigned char  hi, lo;
#endif

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
        // двойное нажатие
        Click_2();
        break;
      case 3:
        // 3 нажатия
        Click_3();
        break;
      case 4:
        // 4 нажатия
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

#ifdef UART_DEBUG
      case 3:
        // 2 коротких + длинное нажатие
        bat = GetBat();
        bat_mV = ((unsigned long)(bat * 1000L) / (unsigned long)BAT_COEF);
        hi = MainPWM_Hi;
        lo = MainPWM_Lo;
        printf("%d %d[mV] %d Hi:0x%X Lo:0x%X\n", bat, bat_mV, GetTemp(), hi, lo);
        Key_Set_RELEASE();
        break;
#endif
      }
    }

    Wait30ms();
    if (Mode == MODE_DO_PW_OFF) {
      // Выключение
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

