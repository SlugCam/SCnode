/*
*  Relay Testing code
*  Will Turn on the pi if motion is detected and is turned off by the msp430's onboard push button
*/


#include <driverlib.h>

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output. and the relay set to off
   digitalWrite(10, HIGH);
   digitalWrite(9, LOW);
   pinMode(10,OUTPUT);
   pinMode(9,OUTPUT);
   
   pinMode(PUSH2, INPUT_PULLUP);
   attachInterrupt(PUSH2, off, FALLING); // Interrupt is fired whenever button is pressed
   
   pinMode(P2_3, INPUT);
   attachInterrupt(P2_3, motion, RISING); // Interrupt is fired when motion is detected

}

// the loop routine runs over and over again forever:
void loop() {
              
   
}

//push button's interupt routine
void off(){
   digitalWrite(10, HIGH);
   digitalWrite(9, LOW);
}

//PIR's interupt routine
void motion(){
  if(digitalRead(10) == HIGH){
         digitalWrite(10, LOW);
         digitalWrite(9, HIGH);
  }  
  
}

//Add SPI interupt here
