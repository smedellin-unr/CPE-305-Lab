/*
 * CPE 301 Lab 7 Sample code
 * Debouncing, 7-Segments, EEPROM
 * Written By Syed Zawad, Fall 2020
 * This program counts the presses of a button and displays the counts on
 * a 7-segment display (displays 0 thru F). The counter variable is also stored and read back from
 * EEPROM memory at boot time. 
 * 
 * Modified by Scott Medellin
 * Date: 7/7/2021
 */

#include <Arduino.h>

#define NUMBER_OF_SEGMENTS 7
#define PIN13 13
#define PIN_OFFSET 2          // Pin offset is necessary because I am using Pins 2 - 8 instead of Pins 0 - 6

uint8_t seg_config[16][8] = {
// a     b     c     d     e     f     g
  {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW }, // 0
  {LOW,  HIGH, HIGH, LOW,  LOW,  LOW,  LOW }, // 1
  {HIGH, HIGH, LOW,  HIGH, HIGH, LOW,  HIGH}, // 2
  {HIGH, HIGH, HIGH, HIGH, LOW,  LOW,  HIGH}, // 3
  {LOW,  HIGH, HIGH, LOW,  LOW,  HIGH, HIGH}, // 4
  {HIGH, LOW,  HIGH, HIGH, LOW,  HIGH, HIGH}, // 5
  {HIGH, LOW,  HIGH, HIGH, HIGH, HIGH, HIGH}, // 6
  {HIGH, HIGH, HIGH, LOW,  LOW,  LOW,  LOW }, // 7
  {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}, // 8
  {HIGH, HIGH, HIGH, LOW,  LOW,  HIGH, HIGH}, // 9
  {HIGH, HIGH, HIGH, LOW,  HIGH, HIGH, HIGH}, // A
  {LOW,  LOW,  HIGH, HIGH, HIGH, HIGH, HIGH}, // B
  {HIGH, LOW,  LOW,  HIGH, HIGH, HIGH, LOW }, // C
  {LOW,  HIGH, HIGH, HIGH, HIGH, LOW,  HIGH}, // D
  {HIGH, LOW,  LOW,  HIGH, HIGH, HIGH, HIGH}, // E
  {HIGH, LOW,  LOW,  LOW,  HIGH, HIGH, HIGH}, // F
};

// make some pointers to a port you want for the 7 segment register

// make some pointers to a different port for the button

// make some pointers to the EEPROM registers
// Look at the EEPROM DEMO CODE

// Globals

// tracks number of times that the button has been pressed
volatile uint8_t counter = 0;
// tracks the last state of the button during the previous cycle
volatile uint8_t last_button_state = 1;
// flag that catches rising edge
volatile bool released = false;
// data to write to EEPROM
uint8_t data_in = 0;
// data to read from EEPROM
uint8_t data_out = 0;
// EEPROM address
uint8_t eeprom_address = 0x0025;

// get a pointer to the EEPROM ADDRESS REGISTER (16-bit)
volatile unsigned int* EEPROM_ADDR_REG = (unsigned int*) 0x41;
// get a pointer to the EEPROM Data Register (8-bit)
volatile unsigned char* EEPROM_DATA_REG = (unsigned char*) 0x40;
// get a pointer to the EEPROM Control Register (8-bit)
volatile unsigned char* EEPROM_CNTRL_REG = (unsigned char*) 0x3F;

// Prototypes
void eeprom_write(unsigned int address, unsigned char data_in);
void display_num(unsigned char num);
unsigned char eeprom_read(unsigned int address);

void setup() 
{
  // Set up the ddr, port registers for input and output ports
  pinMode(PIN13, INPUT_PULLUP);

  for(uint8_t idx_pin = PIN2; idx_pin < NUMBER_OF_SEGMENTS + PIN_OFFSET; idx_pin++) {
    pinMode(idx_pin, OUTPUT);
  }
  // Read counter from EEPROM
  counter = eeprom_read(eeprom_address);
  // initialize display to whatever we read...
  display_num(counter);
}

void loop() 
{
  int push_button_state = digitalRead(PIN13);

  if(push_button_state == LOW)
    delay(100);

  if(push_button_state == HIGH && last_button_state == LOW)
    released = true;

  if(released) {
    if(counter == 16)
      counter = 0;

    eeprom_write(eeprom_address, counter);
    display_num(counter++);
    released = false;
  }

  last_button_state = push_button_state;
}

void display_num(unsigned char num)
{
  for(uint8_t idx_ = PIN2; idx_ < NUMBER_OF_SEGMENTS + PIN_OFFSET; idx_++)
    digitalWrite(idx_, seg_config[num][idx_ - PIN_OFFSET]);
}

void eeprom_write(unsigned int address, unsigned char data_in)
{
  /* Wait for completion of previous write */
  while(*EEPROM_CNTRL_REG & 0x02);
  /* Set up address and Data Registers */
  *EEPROM_ADDR_REG = address;
  *EEPROM_DATA_REG = data_in;
  /* Write logical one to EEMPE */
  *EEPROM_CNTRL_REG |= 0x04;
  /* Write logical zero to EEPE */
  *EEPROM_CNTRL_REG &= ~(0x02);
  /* Write logical one to EEPE */
  *EEPROM_CNTRL_REG |= 0x02;
}
unsigned char eeprom_read(unsigned int address)
{
  /* Wait for completion of previous write */
  while(*EEPROM_CNTRL_REG & 0x02);
  /* Set up address register */
  *EEPROM_ADDR_REG = address;
  /* Start eeprom read by writing EERE */
  *EEPROM_CNTRL_REG |= 0x01;
  /* Return data from Data Register */
  return *EEPROM_DATA_REG;
}