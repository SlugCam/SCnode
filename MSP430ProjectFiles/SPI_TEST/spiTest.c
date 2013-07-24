/*
 *Author: Kevin Abas
 *This is a very basic and simple test for testing SPI communication.
 *Any Data sent on the interupt line will wake up the MSP430 and if
 *'HELLO WORLD' is sent, then an LED is turned on
 */

#include "msp430g2553.h"
#include <string.h>
 
char spiBuf[20]; //string buffer for SPI communication 
char bufIndex=0;
 
void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  P1DIR |= BIT0;
  while (P1IN & BIT4);                   // If clock sig from mstr stays low,
   
  P1SEL = BIT1 + BIT2 + BIT4;
  P1SEL2 = BIT1 + BIT2 + BIT4;
  UCA0CTL1 = UCSWRST;                       //Put state machine in reset
  UCA0CTL0 |= UCMSB + UCSYNC;               // 3-pin, 8-bit SPI master
  UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
  IE2 |= UCA0RXIE;                          // Enable USCI0 RX interrupt
 
  __bis_SR_register(LPM4_bits + GIE);       // Enter Lowest power saving mode, enable interrupts
}

/*SPI Interupt*/ 
__attribute__((interrupt(USCIAB0RX_VECTOR))) void USCI0RX_ISR (void)//way to declare interupts when using msp430-gcc
{
  char value = UCA0RXBUF; //Read SPI register
  if (value == '\n') {
    if (strncmp(spiBuf, "HELLO WORLD", 11) == 0) {
      P1OUT |= BIT0;
    } else {
      P1OUT &= ~BIT0;
    }
    bufIndex = 0;
  } else {
    spiBuf[bufIndex] = value;
    bufIndex++;
  }
 
}