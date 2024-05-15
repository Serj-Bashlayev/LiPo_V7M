/* 
  sample UART software
  transmit serial data at 9600,N,8,1
  code for avr-gcc
           ATTiny85 at 8 MHz


  code in public domain
*/

#define USE_PRINTF
#define UART_PIN    PORTB0

#include <ioavr.h>
#include <stdio.h>
#define uint8_t unsigned char

/* ATTiny85 IO pins
     ___^___
   -|PB5 VCC|-
   -|PB3 PB2|-
   -|PB4 PB1|-
   -|GND PB0|- serial out
     -------
*/

/* prototypes */
// misc routines
//int serial_putc(char c, FILE *file);
void serial_write(uint8_t tx_byte);

// must be volatile (change and test in main and ISR)
volatile uint8_t tx_buzy = 0;
volatile uint8_t bit_index;
volatile uint8_t _tx_buffer; 

/*** ISR ***/

// call every ...us
void TX_ISR(void) { 
  // software UART
  // send data
  if (tx_buzy) {
    if (bit_index == 0) {
      // start bit (= 0)
      PORTB_PORTB0 = 0;
    } else if (bit_index <=8) {
      // LSB to MSB
      if (_tx_buffer & 1) {
        PORTB_PORTB0 = 1;
      } else {
        PORTB_PORTB0 = 0;
      }
      _tx_buffer >>= 1;        
    } else if (bit_index >= 9) {
      // stop bit (= 1)
      PORTB_PORTB0 = 1;
      tx_buzy = 0;
    }
    bit_index++;
  }
}

/*** UART routines ***/
// send serial data to software UART, block until UART buzy
void serial_write(uint8_t tx_byte) {
  while(tx_buzy);
  bit_index  = 0;
  _tx_buffer = tx_byte;
  tx_buzy = 1;
}

#ifdef USE_PRINTF
/*** connect software UART to stdio.h ***/
int putchar(int outChar)
{
  serial_write(outChar);
  return(outChar);
}
#else
void serial_print(const char *str) {
  uint8_t i;
  for (i = 0; str[i] != 0; i++) {
    serial_write(str[i]);
  }
}
#endif

#if 0
/*** misc routines ***/

// safe access to millis counter
uint64_t millis()
{
  uint64_t m;
  cli();
  m = _millis;
  sei();
  return m;
}

/*** main routines ***/

void setup(void)
{
  // LED IO
  sbi(DDRB,  PB3); // set LED pin as output
  sbi(PORTB, PB3); // turn the LED on
  // Software UART IO
  sbi(DDRB,  UART_PIN); // UART_PIN as output
  sbi(PORTB, UART_PIN); // serial idle level is '1'
  /* interrup setup */
  // call ISR(TIM0_COMPA_vect) every 103us (for 9600 bauds)
  // set CTC mode : clear timer on comapre match
  // -> reset TCNTO (timer 0) when TCNTO == OCR0A
  sbi(TCCR0A, WGM01);
  // prescaler : clk/8 (1 tic = 1us for 8MHz clock)
  sbi(TCCR0B, CS01);
  // compare register A at 103 us
  OCR0A = 103;
  // interrupt on compare match OCROA == TCNT0
  sbi(TIMSK, OCIE0A);
  // Enable global interrupts
  sei();
#ifdef USE_PRINTF
  // init stdout = serial
  init_printf();
#endif
}

void loop(void)
{
  // every 100ms toggle LED
  if ((millis() - old_millis) > 2000) {
    // Toggle Port B pin 3 output state
    PORTB ^= 1 << PB3;
    old_millis = millis();
#ifdef USE_PRINTF
    printf("toggle LED\r\n");
#else
    serial_print("toggle LED\r\n");
#endif
  }
}
* /
/*
  Arduino like
*/
int main(void)
{
  setup();
  for (;;) {
    loop();
  }
};
#endif

