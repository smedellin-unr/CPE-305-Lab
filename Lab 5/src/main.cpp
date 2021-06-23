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

/// Globals
//initialize StateMachine variables
volatile uint16_t Step;
volatile uint16_t NextStep = idle;
volatile uint8_t in_char = nul;
volatile uint16_t ticks;

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

void setup() 
{
  // Clear TCCR1A and TCCR1B register
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  // Setting PB-6 as output
  DDRB = (1 << DD6);
  // Setup Serial Port communication
  Serial.begin(9600);
}

void loop() {

  Step = NextStep;

  switch (Step) {

    case stop: {
      PORTB &= ~(1 << PB6);
      Serial.write("Idling...\n");
      NextStep = idle;
      in_char = nul;
      break;
    }

    case idle: {
      if(Serial.available()) {
        in_char = Serial.read();
      }
      if(in_char != q && in_char != nul){
        Serial.write("Decoding...\n");
        NextStep = decode;
        break;
      }
      in_char = nul;
      break;
    }

    case decode: {

      uint16_t last_correct_ticks = ticks;
      ticks = decodeTicks(in_char);
      if(ticks == incorrect_key_stroke) {
        Serial.write("[Err] Incorrect Key Stroke!\n");
        ticks = last_correct_ticks;
        NextStep = play;
        in_char = nul;
        break;
      }
      Serial.write("Playing...\n");
      NextStep = play;
      in_char = nul;
      break;

    }

    case play: {
      // Produce Square wave
      PORTB |= (1 << PB6);
      delay_(ticks);
      PORTB &= ~(1 << PB6);
      delay_(ticks);

      if(Serial.available()) {
        in_char = Serial.read();
      }

      if(in_char == q) {
        Serial.write("Stopping...\n");
        NextStep = stop;
        in_char = nul;
        break;
      }
      else if(in_char != 0) {
        Serial.write("Decoding...\n");  
        NextStep = decode;
        break;
      }
      break;
    }
  }
}