//===========================================================================
// FILE:        key.c
// DESCRIPTION: 
// 
// AUTHOR:      Serj Balabay
// VERSION:     0.1
// DATE:        05/05/2024
// 
// LAST MODIFICATION : Serj Balabay (05.05.2024)
// HISTORY:
//===========================================================================

/** include files **/
#include "hard.h"
#include "misc.h"

/** local definitions **/
#define SHORT_PRESS_TIME 400
#define LONG_PRESS_TIME  800
#define RELEASE_TIME     100

typedef enum {      /*  __/-\______/-------------\______        */
  S_UNPRESS = 0,    /*  --\___/----\_____________/------        */
  S_WAIT_10,        /*  __/-\______/----\_______________        */
  S_WAIT_01,        /*  ____/-\_________________________        */
  S_CLICK,          /*  ______|_________________________        */
  S_LONGPRESS,      /*  ________________/--------\______        */
  S_LONGPRESS_IMP,  /*  ________________|___|___|_______        */
  S_RELEASE         /*  _______/\_________________/\____        */
  /*             Timer   __SSSSR_____SSSSLLL_LLL_LR_____        */
} S_TD;

typedef struct {
  S_TD          State;
  unsigned char Counter;
} KEY_STATE_NEW_TD;

/** default settings **/

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/

/** public functions **/

/** private functions **/

KEY_STATE_NEW_TD Key_State()
{
  static unsigned char counter;
  static S_TD          state;
  unsigned char        key;

  key = KEY_PRESSED() ? 1 : 0;

  switch (state) {

  case S_UNPRESS:
    if (key) {
      state = S_WAIT_10;
      ClearTimer();
    }
    else {
      if (ModeOnOff == STATE_PW_OFF)
        ModeOnOff = STATE_DO_PW_OFF;
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

  return((KEY_STATE_NEW_TD){state, counter});
}



