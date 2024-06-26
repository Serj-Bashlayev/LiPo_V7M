//===========================================================================
// FILE:        key.c
// DESCRIPTION: ��������� ��������� ������� ������
//
// AUTHOR:      Serj Balabay
// VERSION:     0.3
// DATE:        05.05.2024
//
// LAST MODIFICATION : Serj Balabay (12.05.2024)
// HISTORY:
// 05.05.2024 (v0.1):
//     - Initial release
// 06.05.2024 (v0.2):
//     - Added Key_Reset_SM()
// 12.05.2024 (v0.3):
//     - Added Key_Set_RELEASE()
//
//===========================================================================

#include "hard.h"
#include "misc.h"
#include "key.h"

static unsigned char counter;
static S_TD          state;

void Key_Reset_SM(void)
{
  state = S_UNPRESS;
  counter = 0;
}

void Key_Set_RELEASE(void)
{
  state = S_RELEASE;
  ClearTimer();
  counter = 0;
}

KEY_STATE_TD Key_State(void)
{
  unsigned char        key;

  key = KEY_PRESSED() ? 1 : 0;

  switch (state) {

  case S_UNPRESS:
    if (key) {
      state = S_WAIT_10;
      ClearTimer();
    }
    else {
      if (Mode == MODE_PW_OFF)
        Mode = MODE_DO_PW_OFF;
    }
    counter = 0;
    break;

  case S_WAIT_10:
    if (key) {
      if (GetTimer() > LONG_PRESS_TIME) {
        state = S_LONGPRESS_IMP;
        counter += 1;
      }
    }
    else {
      counter += 1;
      state = S_WAIT_01;
    }
    break;

  case S_WAIT_01:
    if (key) {
      state = S_WAIT_10;
      ClearTimer();
    }
    else {
      if (GetTimer() > SHORT_PRESS_TIME) {
        state = S_CLICK;
      }
    }
    break;

  case S_CLICK:
    state = S_UNPRESS;
    ClearTimer();
    break;

  case S_LONGPRESS_IMP:
    if (key) {
      ClearTimer();
      state = S_LONGPRESS;
    }
    else {
      state = S_RELEASE;
      ClearTimer();
    }
    break;

  case S_LONGPRESS:
    if (key) {
      if (GetTimer() > LONG_PRESS_TIME) {
        state = S_LONGPRESS_IMP;
      }
    }
    else {
      state = S_RELEASE;
      ClearTimer();
    }
    break;

  case S_RELEASE:
    if (!key) {
      if (GetTimer() > RELEASE_TIME) {
        state = S_UNPRESS;
      }
    }
    else {
      ClearTimer();
    }
    break;
  }

  return((KEY_STATE_TD){state, counter});
}



