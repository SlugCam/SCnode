/*
 *Author: Kevin Abas
 *This is a very basic and simple test for testing SPI communication.
 *Any Data sent on the interupt line will wake up the MSP430 and if
 *'HELLO WORLD' is sent, then an LED is turned on
 */

#include <msp430g2553.h>
 
void main() {
  WDTCTL = WDTPW + WDTHOLD; // get rid of watchdog timer
  P1DIR = BIT0; // set LED1 and LED2 to OUT
  P1IES &= ~BIT5; // set edge initially to low-high
  P1IFG &= ~BIT5; // prevent immediate interrupt
  P1IE |= BIT5; 
  _BIS_SR(LPM4_bits + GIE); // Interupts enabled and Lowest power mode
   
}

/*PIR Interupt*/ 
__attribute__((interrupt(PORT1_VECTOR))) void P1_ISR (void)//way to declare interupts when using msp430-gcc
{  
  if ((P1IFG & BIT5) == BIT5) { // if pir is detecting something
    P1IFG &= ~BIT5; // clear interrupt flag
    P1OUT ^= BIT0; // toggle LED1
    P1IES ^= BIT5; // toggle edge
  } else {
    P1IFG = 0; // clear all other flags to prevent infinite interrupt loops
  }
}