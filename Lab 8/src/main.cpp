#include <Arduino.h>

#define MAX_BITS_IN_REGISTER 8

void adc_init() {
  // Set reference voltage to voltage supply and left justify the output
  ADMUX |= (1<<REFS0) | (1 << ADLAR);
  // Set prescaler division  factor oto 128
  ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

uint16_t adc_read(uint8_t adc_channel) {
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

void setup() {
  adc_init();

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}