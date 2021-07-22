// Written by Scott Medellin

#include <Arduino.h>
#include <math.h>

#define systick 0.0625 // microsecond units

// Steps in sequence
#define init 1000
#define decode 2000
#define play 3000
#define stop 4000
#define idle 5000

// Key Strokes
#define A 65
#define B 66
#define C 67
#define D 68
#define E 69
#define F 70
#define G 71

#define a 97
#define b 98
#define c 99
#define d 100
#define e 101
#define f 102
#define g 103

#define q 113
#define nul 0

#define incorrect_key_stroke 9999

#define TBE 0x20
#define MAX_CHAR_ARRAY_SIZE 4
#define RDA 0x80

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

/// Globals
//initialize StateMachine variables
volatile uint16_t Step;
volatile uint16_t NextStep = idle;
volatile uint8_t in_char = nul;
volatile uint16_t ticks = 17000;
volatile bool init_prog = true;
volatile bool error = false;
volatile uint8_t last_char = nul;

// lookup table of Period in microseconds
double frequency_selection[] = {
  2262.727273,                    // 440 Hz A
  2135.922747,                    // 466 Hz A#
  2014.291498,                    // 494 Hz B
  1902.045889,                    // 523 Hz C
  1795.054152,                    // 554 Hz C#
  1693.577513,                    // 587 Hz D
  1592.564103,                    // 624 Hz D#
  1507.450683,                    // 659 Hz E
  1422.664756,                    // 698 Hz F
  1341.351351,                    // 740 Hz F#
  1265.510204,                    // 784 Hz G
  1193.369434,                    // 831 Hz G#
};

uint16_t decodeTicks(uint8_t KeyStroke) {

  switch (KeyStroke) {
    case A: {
      return ((frequency_selection[0]/systick)/2); // 440 Hz
    }
    case B: {
      return ((frequency_selection[2]/systick)/2); // 494 Hz
    }
    case C: {
      return ((frequency_selection[3]/systick)/2); // 523 Hz
    }
    case D: {
      return ((frequency_selection[5]/systick)/2); // 587 Hz
    } 
    case E: {
      return ((frequency_selection[7]/systick)/2); // 659 Hz
    }
    case F: {
      return ((frequency_selection[8]/systick)/2); // 698 Hz
    }
    case G: {
      return ((frequency_selection[10]/systick)/2);// 784 Hz
    }
    case a: {
      return ((frequency_selection[1]/systick)/2); // 440 Hz
    }
    case c: {
      return ((frequency_selection[4]/systick)/2); // 440 Hz
    }
    case d: {
      return ((frequency_selection[6]/systick)/2); // 440 Hz
    }
    case f: {
      return ((frequency_selection[9]/systick)/2); // 440 Hz
    }
    case g: {
      return ((frequency_selection[11]/systick)/2); // 440 Hz
    }
    case nul: {                                     // null value when not typing. Ignore
      break;
    }
    default: {
      return incorrect_key_stroke;                 // incorrect selection;
    }
  }
  
}

// Prototypes
void delay_(uint16_t ticks);
void U0init(unsigned long U0baud);
unsigned char U0kbhit();
unsigned char U0getchar();
void U0putchar(unsigned char U0pdata);

ISR(TIMER1_OVF_vect) {
  TCNT1 = ticks;
  PORTB ^= (1 << PB6);  
}


void setup() 
{
  // Clear TCCR1A and TCCR1B register
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  // enable timer1 overflow interrupt
  TIMSK1 |= (1 << TOIE1);
  // Enable interrupts
  sei();
  // Setting PB-6 as output
  DDRB = (1 << DD6);
  // Setup Serial Port communication
  Serial.begin(9600);
  // custom UART serial send init
  U0init(9600);
}

void loop() {
  /*
  if(Serial.available()) {
    in_char = Serial.read();
  }
  */
  // receive character
  char hex[MAX_CHAR_ARRAY_SIZE] = {'0' ,'x'};
  while (U0kbhit()==0){}; // wait for RDA = true
  in_char = U0getchar();    // read character
  itoa(in_char, hex + 2, 16); // write the integer to ACII string conversion to the char array with offset of 2
  last_char = in_char;
  if (in_char == q) {
      TIMSK1 &= ~(1 << TOIE1);
      PORTB &= ~(1 << PB6);
  }
  else if (in_char != nul && in_char != q) {
    // decode ticks
    ticks = decodeTicks(in_char);
    if (ticks == incorrect_key_stroke) {
      Serial.println("Incorrect keystroke.");
      delay(1000);
    }
    else {
      TIMSK1 = (1 << TOIE1);
    }
  }
}

void delay_(uint16_t ticks) {
  // stop the timer (set prescaler to 0)
  TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));
  // set the counts
  TCNT1 = (uint16_t)(65536 - ticks);
  // start the timer (set prescaler to 1)
  TCCR1B |= (1 << CS10);
  // wait for overflow
  while((TIFR1 & (1 << TOV1)) == 0);
  // stop the timer (set prescaler to 0)
  TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));
  // reset TOV flag
  TIFR1 |= (1 << TOV1);
}

//
// function to initialize USART0 to "int" Baud, 8 data bits,
// no parity, and one stop bit. Assume FCPU = 16MHz.
//
void U0init(unsigned long U0baud)
{
//  Students are responsible for understanding
//  this initialization code for the ATmega2560 USART0
//  and will be expected to be able to intialize
//  the USART in differrent modes.
//
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20; // 0b00100000  <- This sets the Data Register Empty to override any data in URD0 (effectively clearing it)
 *myUCSR0B = 0x18; // 0b00011000  <- These enable receive and transmit
 *myUCSR0C = 0x06; // 0b00000110  <- Sets an 8 bit Character size for receive and transmission
 *myUBRR0  = tbaud;
}
//
// Read USART0 RDA status bit and return non-zero true if set
//
unsigned char U0kbhit()
{
  if((*myUCSR0A & RDA)) {
    return true;
  }
  return false;
}
//
// Read input character from USART0 input buffer
//
unsigned char U0getchar()
{
  return *myUDR0;
}
//
// Wait for USART0 TBE to be set then write character to
// transmit buffer
//
void U0putchar(unsigned char U0pdata)
{
  while(!(UCSR0A & TBE));
  *myUDR0 = U0pdata;
}
