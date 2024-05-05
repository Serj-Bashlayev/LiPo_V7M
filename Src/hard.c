//===========================================================================
// FILE:      hard.c
// AUTHOR:    AVSel
// VERSION:   7.0m
// DATE:      05/03/2014
// LAST MODIFICATION : Serj Balabay (17.04.2020)
//===========================================================================

#include "hard.h"
#include <intrinsics.h>

#define ADC_BATVOLTAGE 0x81 // Internal 1.1V Voltage Reference. Single Ended Input ADC1 (PB2)
#define ADC_TEMP 0x8F       // Internal 1.1V Voltage Reference. Single Ended Input ADC4 (For temperature sensor)

volatile unsigned short Timer;
volatile unsigned char  WaitCount;

unsigned char  ADC_Count, PWM_Count;
UniShort       TempADC, BatADC;
unsigned long  VOut;
unsigned short MainPWM, BatIntr, BatDiv;
unsigned char  MainPWM_Hi, MainPWM_Lo;
signed char    BatIntrLow;

#define MainPWM_MAX 2040  // 0x7F8

#pragma vector = PCINT0_vect
__interrupt void ext0_isr(void)
{
}

// desired value: 1000H
void timer0_init(void)
{
  TCCR0A = 0x00; //stop
  TCNT0  = 0x00; //set count
  OCR0A  = SYS_CLK / 64 / 1000 - 1;
  TCCR0B = 0x03; // Clock Select: CLK/64 (From prescaler)
  TCCR0A = 0x02; // Timer/Counter Mode: CTC
}

// 1000 Hz
#pragma vector = TIMER0_COMPA_vect
__interrupt void timer0_isr(void)
{
  Timer++;
  if (WaitCount)
    WaitCount--;
  if (KEY_PRESSED())
    KEY_PRESSED_ISR = 1;
}

void ClearTimer(void)
{
  TCCR0A = 0x00;  //stop
  __disable_interrupt();
  TCNT0  = 0x00;
  Timer = 0;
  TCCR0A = 0x02;  // Timer/Counter Mode: CTC
  __enable_interrupt();
}

unsigned short GetTimer(void)
{
  unsigned short t;
  __disable_interrupt();
  t = Timer;
  __enable_interrupt();
  return t;
}

void Delay(unsigned short  ms)
{
  ClearTimer();
  while (ms > GetTimer())
    Sleep();
}

void Wait30ms(void)
{
  WaitCount = 30;
  while (WaitCount)
    Sleep();
}

void timer1_init(void)
{
  TCCR1 = 0x00;
  PLLCSR = 0x00;
  OCR1C = 255;
  OCR1A = 0x00;
  TCNT1 = 0x00;
  GTCCR = 0x00;
  // Pulse Width Modulator A Enable
  // OC1A (PB1) cleared on compare match. Set when TCNT1 = $00. ~OC1A (PB0) not connected.
  // Clock Select prescaling - CK (Fisr = 7812 HZ)
  TCCR1 = 1 << PWM1A | 1 << COM1A1 | 1 << CS10;
}

void timer1_sleep(void)
{
  OCR1A = 0x00;
  TCCR1 = 0x00;
}

// 7874 Hz (127µS)
#pragma vector = TIMER1_OVF_vect
__interrupt void timer1_isr(void)
{
  register unsigned char b;
  b = PWM_Count;
  b++;
  if (b >= 8)
    b = 0;
  PWM_Count = b;
  if (b < MainPWM_Lo) {
    b = MainPWM_Hi;
    b++;
    OCR1A = b;
  }
  else {
    b = MainPWM_Hi;
    OCR1A = b;
  }
}

#define SET_TAB_VOUT(v) ((unsigned long)(v * MainPWM_MAX * BAT_COEF))

__flash unsigned long TabVOut[] = {
  SET_TAB_VOUT(2.92), /* 2000 mA  */
  SET_TAB_VOUT(0.84), /*  600 mA  */
  SET_TAB_VOUT(0.24), /*  170 mA  */
  SET_TAB_VOUT(0.07), /*   50 mA  */
  SET_TAB_VOUT(0.02), /*   12 mA  */
  3                   /*    4 mA  */
};

void VOutSet(BRIGHT_TD bright)
{
/*
  if (bright > BRIGHT_ULOW2)
    bright = BRIGHT_ULOW2;
  VOut = TabVOut[bright];
  BatIntr = GetBat();
  PORTB |= 1<<PORTB4;
*/
  unsigned long  vo;
  unsigned short bat;

  if (bright > BRIGHT_ULOW2) {
    bright = BRIGHT_ULOW2;
  }
  vo = TabVOut[bright];
  bat = GetBat();
  __disable_interrupt();
  VOut = vo;
  BatIntr = bat;
  __enable_interrupt();
  PORTB |= 1 << PORTB4;
}

void LedOnFast(BRIGHT_TD bright)
{
  VOutSet(bright);
}

void LedOnSlow(BRIGHT_TD bright)
{
  unsigned long vo,
                x;
  VOutSet(bright);
  if (bright < BRIGHT_ULOW2) {
    vo = VOut;
    __disable_interrupt();  ///
    VOut = SET_TAB_VOUT(0.006);
    __enable_interrupt();   ///
    while (vo > VOut)
    {
      x = VOut / 9 + vo / 70;
      __disable_interrupt();
      VOut += x;
      __enable_interrupt();
    }
    __disable_interrupt();  ///
    VOut = vo;
    __enable_interrupt();   ///
  }
}

// Время dыполнения ~480 µs
void AdjPower(void)
{
  if ((PORTB & (1 << PORTB4)) == 0) {
    MainPWM = 0;
    MainPWM_Lo = MainPWM_Hi = 0;
    return;
  }
  if (VOut < 8) {
    MainPWM = VOut;
  }
  else {
    if (BatIntr > BatADC.s) {
      BatIntrLow--;
      if (BatIntrLow < -20) {
        BatIntr--;
        BatIntrLow = 0;
      }
    }
    else if (BatIntr < BatADC.s) {
      BatIntrLow++;
      if (BatIntrLow > 20) {
        BatIntr++;
        BatIntrLow = 0;
      }
    }

    if ((BatIntr > BatDiv + 5) || (BatIntr + 2 < BatDiv)) {
      BatDiv = BatIntr;
    }

    if (BatDiv == 0) {
      MainPWM = 0;
    }
    else {
      MainPWM = VOut / BatDiv;
    }
  }
  if (MainPWM > MainPWM_MAX)
    MainPWM = MainPWM_MAX;
  __disable_interrupt();
  MainPWM_Lo = MainPWM & 0x07;
  MainPWM_Hi = MainPWM >> 3;
  __enable_interrupt();
}

void LedOff(void)
{
  PORTB &= ~(1 << PORTB4);
}

void LedSysBlink(void)
{
  LedOnFast(BRIGHT_SYSTEM);
  Delay(150);
  LedOff();
  Delay(400);
}

unsigned short GetTemp(void)
{
  unsigned short r;
  __disable_interrupt();
  r = TempADC.s;
  __enable_interrupt();
  return r;
}

unsigned short GetBat(void)
{
  unsigned short r;
  __disable_interrupt();
  r = BatADC.s;
  __enable_interrupt();
  return r;
}

void adc_init(void)
{
  // Analog Comparator Disable
  ACSR  = 1 << ACD;
  ADMUX = ADC_TEMP;
  // ADEN: ADC Enable
  // ADIE: ADC Interrupt Enable
  // ADPS[2:0]: ADC Prescaler Select Bits [100] => Division Factor 16 (125kHz Fcpu 2MHz)
  ADCSRA = 1 << ADEN | 1 << ADIE | 1 << ADPS2;
  ADCSRB = 0x00;
  // ADC Start Conversion
  ADCSRA |= (1 << ADSC);
}

void adc_sleep(void)
{
  ADCSRA = 0;
}

#pragma vector = ADC_vect
__interrupt void adc_isr(void)
{
  if (ADC_Count) {
    ADC_Count--;
    goto Llex;
  }
  ADC_Count = 1;
  if (ADMUX == ADC_TEMP) {
    // Измерение температуры
    ADMUX = ADC_BATVOLTAGE;
    TempADC.c[0] = ADCL;
    TempADC.c[1] = ADCH;
  }
  else {
    // Измерение напряжения
    ADMUX = ADC_TEMP;
    BatADC.c[0] = ADCL;
    BatADC.c[1] = ADCH;
    __enable_interrupt();
    AdjPower();
  }
Llex:;
  // ADC Start Conversion
  ADCSRA |= (1 << ADSC);
}


void WriteEEPROMByte(unsigned char adress, unsigned char byte)
{
  unsigned char savedSREG;
  while (EECR & 0x02);
  EEAR = adress;          // set address
  EEDR = byte;            // set data
  savedSREG = SREG;       // keep setting so it can be restored
  __disable_interrupt();
  EECR |= 1 << EEMPE;     // set "write enable" bit
  EECR |= 1 << EEPE;      // set "write" bit
  SREG = savedSREG;       // restore SREG
  EEAR = 0;
}

unsigned char ReadEEPROMByte(unsigned char adress)
{
  while (EECR & 0x02);
  EEAR = adress;          // set address
  EECR |= 1 << EERE;      // set "read enable" bit
  EEAR = 0;
  return (EEDR);
}

void PowerOff(void)
{
  // Power Off
  __disable_interrupt();
  adc_sleep();
  timer1_sleep();
  TCCR0A = 0x00;          // stop
  PORTB = 1 << PORTB3;    // pull-up PB3 (Key)
  PCMSK = 1 << PCMSK3;    // pin change interrupt enable PB3 (Key)
  GIFR = 1 << INTF0 | 1 << PCIF; // reset interrupt flags
  GIMSK = 1 << PCIE;      // enable pin change interrupt
  // BOD disable, Sleep Enable, Sleep Mode - Power-down
  MCUCR = 1 << BODS | 1 << SE | 1 << SM1 | 1 << BODSE;
  MCUCR = 1 << BODS | 1 << SE | 1 << SM1 | 0 << BODSE;
  __enable_interrupt();
  __sleep();
 
  // Power On
  __disable_interrupt();
  GIMSK = 0x00;
  MCUCR = 0x00;
  PORTB = 1 << PORTB3 | 1 << PORTB0; // pull-up PB3 (Key), +U to R2 (Ubat)
  timer0_init();
  timer1_init();
  adc_init();
  __enable_interrupt();
}

void Sleep(void)
{
  __disable_interrupt();
  MCUCR = 1 << SE;        // Idle mode
  __enable_interrupt();
  __sleep();
}

//call this routine to initialize all peripherals
void InitHard(void)
{
  __disable_interrupt();
  CLKPR = 1 << CLKPCE;  // Clock Prescaler Change Enable
  CLKPR = 1 << CLKPS1;  // Clock Division Factor = 4 (CLOCK CPU = 2 MHz)
  GTCCR = 0x00;         // General Timer/Counter Control Register
  PRR = 1 << PRUSI;     // shuts down the USI by stopping the clock to the module
  // Timer/Counter1 Overflow Interrupt Enable
  // OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
  TIMSK = 1 << OCIE0A | 1 << TOIE1;
  DDRB = 1 << DDB4 | 1 << DDB1 | 1 << DDB0;
  DIDR0 = 1 << ADC1D;   // ADC1 Digital Input Disable
  // 04.02.2020: add and check
  ACSR = 1 << ACD;      // Analog Comparator Disable
  PowerOff();
}

