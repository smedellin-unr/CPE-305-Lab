#include <Arduino.h>

#define MAX_BITS_IN_REGISTER 8
#define THRESHOLD 85
#define AI_PORT0 0

void adc_init() {
  // Set reference voltage to voltage supply and left justify the output
  ADMUX |= (1<<REFS0) | (1 << ADLAR);
  // Set prescaler division  factor oto 128
  ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

uint16_t adc_read(uint8_t adc_channel) {
  // Clears DIDR0 and DIDR2
  DIDR0 = 0x00;
  DIDR2 = 0x00;
  // Disables appropriate digital input buffer
  if (adc_channel < MAX_BITS_IN_REGISTER)
    DIDR0 |= (1 << adc_channel);
  else 
    DIDR2 |= (1 << (adc_channel - MAX_BITS_IN_REGISTER));
  // Start sample
  ADCSRA |= (1 << ADSC);
  // Wait till sampling complete
  while(ADCSRA & (1 << ADIF) == 0);
  // return ADC result
  return ADCH;
}

// Globals
volatile uint8_t result = 0;

void setup() {
  adc_init();
  // Set PB7 (GPIO pin 13) as output
  DDRB |= (1 << PB7);

}

void loop() {
  result = adc_read(AI_PORT0);
  if(result < THRESHOLD) PORTB |= (1 << PORTB7);
  else PORTB &= ~(1 << PORTB7);
}