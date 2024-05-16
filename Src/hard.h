//===========================================================================
// FILE:      hard.h
// AUTHOR:        AVSel
// VERSION:     7.5m
// DATE:    05/03/2014
// LAST MODIFICATION : Serj Balabay (16.05.2024)
//===========================================================================

#ifndef _HARD_H
#define _HARD_H

#include <ioavr.h>

#if (__VER__ == 610)
#define TIMER1_COMPA_vect TIM1_COMPA_vect
#define TIMER1_OVF_vect TIM1_OVF_vect
#define EEMPE EEMWE
#define EEPE  EEWE
#endif

#ifndef BODS
#define BODS 7U
#endif

#ifndef BODSE
#define BODSE 2U
#endif

//#define SYS_CLK 1000000L
#define SYS_CLK 2000000L

typedef enum
{
  BRIGHT_ULOW2 = 0, /* 4mA    */
  BRIGHT_ULOW1,     /* 12mA   */
  BRIGHT_LOW,       /* 50mA   */
  BRIGHT_MID,       /* 170mA  */
  BRIGHT_HI,        /* 600mA  */
  BRIGHT_UHI        /* 2000mA */
} BRIGHT_TD;

#define BRIGHT_SYSTEM   BRIGHT_LOW

//#define BAT_COEF 216 // R4/(R4+R2)*1024/1.1
#define BAT_COEF 221 // реальный коэфф. определённый по измерениям, переданным по UART
#define BAT_SHOTTKY 0.2 // падение напряжения на диоде Шоттки

#define BAT_CALC(v) (unsigned short)(BAT_COEF * (v - BAT_SHOTTKY))

#define T_OFFSET (275 + 10) // 275 - const, +10 - коррекция определённая по измерениям, переданным по UART

typedef union {
  unsigned short s;
  unsigned char  c[2];
} UniShort;


void InitHard(void);

void ClearTimer(void);
unsigned short GetTimer(void);
void Delay(unsigned short  ms);
void Wait30ms(void);

void LedOnFast(BRIGHT_TD mode);
void LedOnSlow(BRIGHT_TD mode);
void LedOff(void);
void LedSysBlink(void);

#define KEY_PRESSED() ((PINB & 0x08) == 0)
#define KEY_PRESSED_ISR  GPIOR0_Bit0

signed char GetTemp(void);
unsigned short GetBat(void);

void Sleep(void);
void PowerOff(void);

void WriteEEPROMByte(unsigned char adress, unsigned char byte);
unsigned char ReadEEPROMByte(unsigned char adress);

#endif

