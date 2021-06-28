//
// Program to echo serial input characters from
// Putty keyboard back to Putty display.
// This version uses proper pointers.
//
//Written by D. Egbert, Version 1.5, 04/03/2017
//
#include <Arduino.h>
#include <stdlib.h>

#define RDA 0x80
#define TBE 0x20
#define MAX_CHAR_ARRAY_SIZE 4

#define nul 0x00

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

// Prototypes
void U0init(unsigned long U0baud);
unsigned char U0kbhit();
unsigned char U0getchar();
void U0putchar(unsigned char U0pdata);
//
// main()
//
void setup()
{
 // initialize the serial port on USART0:
 U0init(9600);
}
void loop()
{
  unsigned char cs1;
  char hex[MAX_CHAR_ARRAY_SIZE] = {'0' ,'x'};
  while (U0kbhit()==0){}; // wait for RDA = true
  cs1 = U0getchar();    // read character
  itoa(cs1, hex + 2, 16); // write the integer to ACII string conversion to the char array with offset of 2
  uint8_t counter = 0;
  // Transmit character one at a time over USART comms
  while(counter < MAX_CHAR_ARRAY_SIZE) {
    U0putchar(hex[counter++]);
  }
  // Transmit new line character at the end of the payload
  cs1 = '\n';
  U0putchar(cs1);
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
 *myUCSR0A = 0x20; // 0b00100000
 *myUCSR0B = 0x18; // 0b00011000
 *myUCSR0C = 0x06; // 0b00000110
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
