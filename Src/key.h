#ifndef __KEY_H_INCL__
#define __KEY_H_INCL__

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
} KEY_STATE_TD;

#define KEY_PRESSED() ((PINB & 0x08) == 0)
#define KEY_PRESSED_ISR  GPIOR0_Bit0

void Key_Reset_SM(void);
void Key_Set_RELEASE(void);
KEY_STATE_TD Key_State(void);

#endif // __KEY_H_INCL__

