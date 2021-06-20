// Written by Scott Medellin

#include <Arduino.h>

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

void setup() 
{
  // Clear TCCR1A and TCCR1B register
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  // Setting PB-6 as output
  DDRB = (1 << DD6);
}

void loop() 
{

  PORTB |= (1 << PB6);
  delay_(10000);
  PORTB &= ~(1 << PB6);
  delay_(10000);
}