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
//    �������� ���� �� AVSel � ������ ��������
//
// 7.1m - (17.04.2020)
//    �������������� ���������
//
// 7.2m - (06.05.2024)
//    ����������� � ������� � ��������� ���� ������ ��������� ������� ������.
//    ��������� ������ ������������ ���������� �� ������� ����.
//
// 7.3m - (06.05.2024) 
//    � �������� ������ (��������� �������� ��������) � ������� ���������� �� ��� �� �������, �� ������� ��� ��������.
//    ����� ��������� ���������� ���������� (����������� �����) - ��� ���������� ������ (����������� �������������� ���������� �������).
//
// 7.4m - (12.05.2024)
//    ����� ������ �� ������ ������ ������� ���������� ������ (����������� �������������� ���������� �������).
//
// 7.5m - (16.05.2024)
//    ��� ������� ��������� ����������� ������ ���������� ������������ � ����������� �� UART (pin PB0, 7874 bits/s).
//    ����� � UART - ������� ���� � ����������. (UART ���������� ������������ ������� UART_DEBUG � ���������� �����������)
//    - ����� ��������� �������������� BAT_COEF. �������� ��������� ���������� +-25 ��
//    - ����� ��������� �������������� T_OFFSET. �������� ��������� ����������� +-2�C
//    - ��������� ������� �� ����� ������ ���������� � ������ BAT_CALC(v)
//    - GetTemp() ���������� ����������� � ��������.
//    - TempOff � EEPROM ����������� � ��������
//    - ��������� ����������� �������� ������� ��������� � 75�� �� 65��.
//===========================================================================

#include "hard.h"
#include "misc.h"
#include "key.h"
#include <stdio.h>

MODE_TD       Mode;      // �����
BRIGHT_TD     Mode1_Bright, Mode2_Bright, Mode3_Bright; // ������� ������
BRIGHT_TD     Bright;    // ������� �������
unsigned char PowerOnBlocked; // ��������� �������� �������������


// ����� ������
// ������ ������� 10, 4 ��� 1 ���
// ����� ������� 0.1 ���
void BlinkMode(void)
{
  unsigned char i,
                j;
  LedOff();
  Delay(500);
  LedOnFast(Bright);
  KEY_PRESSED_ISR = 0;  // ����� ����� ������� ������
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

// �������� �������
// PW_OFF: ��������� � ������ 1
// PW_ON:  ����������
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
      // � ������ ������:
      // ��������� �������� �������� �� ��� �� �������, �� ������� ���������
      Mode1_Bright = Bright;
    }
    Mode = MODE_DO_PW_OFF;
  }
}

// ��������� 0.8 ���
// PW_OFF: ��������� � ������ 2
// PW_ON:  ���������� �������
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

// ������� �������
// PW_OFF: ��������� � ������ 3
// PW_ON:  ���������� �������
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

// 3 �����
// PW_OFF: ��������� ���������� ������������
// PW_ON:  ����� ������
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

// 4 �������
// PW_OFF: ����������/������������� ��������� ��������
// PW_ON:  ���������� ������� ������� � EEPROM
void Click_4(void)
{
  switch (Mode) {
  case MODE_PW_OFF:
    // ����������/������������� ��������� ��������
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
    // ���������� ������� ������� � EEPROM
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

// ���� + ������� � ����������
// PW_OFF: ��������� ����� 6 ��� - ����� ���������� �����������
// PW_ON:  ���������� ������� � �������� ��� ���������
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
        // �������� �������
        Click();
        break;
      case 2:
        // ������� �������
        Click_2();
        break;
      case 3:
        // 3 �������
        Click_3();
        break;
      case 4:
        // 4 �������
        Click_4();
        break;
      }
      break;
    case S_LONGPRESS_IMP:
      switch (KeyState.Counter) {
      case 1:
        // ������� �������
        LongPress();
        break;
      case 2:
        // �������� + ������� �������
        DblLongPress();
        break;

#ifdef UART_DEBUG
      case 3:
        // 2 �������� + ������� �������
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
      // ����������
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

