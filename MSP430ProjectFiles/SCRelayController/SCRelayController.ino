/*
 Slugcam MSP430 Firmware
 
 Date: 2/22/2015
 Author: Kevin Abas
 Last Modified: 6/2/2015
 File: 
 
 This program allows the MSP430 the ability to monitor the passive 
 infared sensor for motion, and to power on the SlugCam system using 
 a dual coil relay. SlugCam also uses the MSP430 to then keep the SlugCam 
 system awake by using an MSP430 on board timer(32Khz crystal). SlugCam 
 communicates how long it wishes to stay awake using serial SPI 
 communication. Should motion be a nusence, SlugCam also has the ability 
 to tell the MSP430 to ignore the motion interupts for a longer period 
 of time. Also using the msp430's available RTC clock library the msp430
 can also act as the systems clock by monitoring and syncing it.
 The MSP430 expects the following 10 byte packets when its slave select 
 pin is active (note the \n is the deliminating LF):
 
     Set Time:
         'T000000\n__' (T+hrs(__)+min(__)+sec(__)+\n__)
     Set Date:
         'D00000000\n' (D+yr(____)+mon(__)+day(__)+\n)
     Set Sleep Countdown:
         'S000000\n__' (S+hrs(__)+min(__)+sec(__)+\n__)
     Special Commands:
         'CON\n______' (C+remainOn(ON)+\n______)  <-Stay on forever
         'CDP0000\n__' (C+disable(__)+disableSec(__)+\n__) <-ignore PIR for this long
        
     *NOTE: The Pi should replace the trailing _'s with 0's 
 
 GPIO Config:
   PIR Sensor:
     DOUT: pin 11 (P2.3
     EN: pin 12 (P2.4)
   SPI:
     MISO: pin 14 (P1.6) (UCB0SOMI)
     MOSI: pin 15 (P1.7) (UCB0SIMO)
     
     CE0: pin 6 (P1.4) (Slave Select)
     SCLK: pin 7 (P1.5)  (UCB0CLK)
     
   Relay:
     Set: pin 9 (P2.1)
     Reset: pin 10 (P2.2)
 */
#include "msp430g2553.h"
#include <string.h>
#include <RTCplus.h> // Real-Time-Clock Library

//Interupt Service Routine Flags
volatile int motion_interupt_flag  = LOW;
volatile int spi_interupt_flag     = LOW;
volatile int spi_error_flag        = LOW;
volatile int timer_interupt_flag   = LOW;
volatile int timer_running_flag    = LOW;
volatile int disable_pir_flag    = LOW;

//Protected Timer Values
volatile unsigned int end_timer_seconds   = 99;
volatile unsigned int end_timer_minutes   = 99;
volatile unsigned int end_timer_hours     = 99;
volatile unsigned int disable_pir_seconds   = 99;
volatile unsigned int disable_pir_minutes   = 99;


//Relay and PIR pins
const int relay_set_pin   = 9;
const int relay_reset_pin = 10;
const int pir_dout_pin    = 11; 
const int pir_enable_pin  = 12;

//RTC Clock Data Structure
RealTimeClock my_clock;

//SPI RX Buffer Variables
char SPI_RX_buff[13];
volatile char SPI_TX_Error[16] = "Invalid\n";
char SPI_RX_index = 0;
volatile char SPI_TX_index = 0;
int  num_seconds, num_minutes, num_hours, 
      num_year, num_month, num_day;
char *rx_seconds, *rx_minutes, *rx_hours, 
      *rx_year, *rx_month, *rx_day;

/******************************
    Init Config Function
******************************/
void setup() {

  // initialize the pins for I/O
  pinMode(relay_set_pin, OUTPUT);
  pinMode(relay_reset_pin, OUTPUT);
  pinMode(pir_dout_pin, INPUT);
  pinMode(pir_enable_pin, OUTPUT);

  attachInterrupt(pir_dout_pin, motion_detected, RISING);
  digitalWrite(pir_enable_pin, HIGH);

  disable_wdt(); // Watch Dog Timer
  
  setup_spi();
  
  _BIS_SR(GIE); //Enable Global Interupts
  my_clock.begin(); //Start RTC
  set_sleep_timer(0, 5, 0); //auto sleep countdown is 5 min
  
}


/******************************
        Main Loop
******************************/
void loop() {
  
  while (P1IN & BIT5);                   // If clock sig from mstr stays low,
                                            // it is not yet in SPI mode
  flash_spi_detected();                 // Blink 3 times
  
  if(timer_interupt_flag == HIGH){
    process_timer_expired();
    timer_interupt_flag = LOW; 
  }else if(spi_interupt_flag == HIGH){  
    process_spi();
  }else if(motion_interupt_flag == HIGH){
    motion_interupt_flag = LOW;  
    process_motion();
  }else if( (motion_interupt_flag == LOW) &&
             (timer_interupt_flag == LOW) &&
               (spi_interupt_flag == LOW) ){
    __bis_SR_register(LPM3_bits + GIE);       // Enter LPM3, enable interrupts
  }
   
}

/******************************
  Interupt Handling Functions
******************************/
void process_motion(void){
  //If relay isn't on turn it on
  if(digitalRead(relay_set_pin) == LOW){
       timer_running_flag = HIGH;
       digitalWrite(relay_set_pin, HIGH);
       digitalWrite(relay_reset_pin, LOW);
  }
  set_sleep_timer(0, 5, 0); //auto sleep countdown is 5 min
  timer_running_flag    = HIGH;
  digitalWrite(pir_enable_pin, LOW); //disable PIR to save power
}

void process_spi(void){
  
  switch(SPI_RX_buff[0]){ 
    
     case 'T': //set RTC time
       get_buffer_time();
       my_clock.Set_Time(num_hours, num_minutes, num_seconds);
       
     case  'D': //set Date
       get_buffer_date();
       my_clock.Set_Date(num_year, num_month, num_day);
       
     case  'S': //set sleep countdown
       get_buffer_time(); 
       set_sleep_timer(num_seconds, num_minutes, num_hours);
       timer_running_flag    = HIGH;
       digitalWrite(pir_enable_pin, LOW);
       
     case  'C': //special command (stay on | diable PIR)
       if(SPI_RX_buff[1] == 'O') {
         digitalWrite(pir_enable_pin, LOW);
          end_timer_seconds   = 99;
          end_timer_minutes   = 99;
          end_timer_hours     = 99;
       }else if(SPI_RX_buff[1] == 'D') {
          get_disable_time();
          disable_pir_seconds = (my_clock.RTC_sec + num_seconds ) % 60;
          if( (end_timer_seconds <= 30 && ( num_seconds >= 30 || my_clock.RTC_sec >= 30 ) )
              || ( num_seconds >= 30 && my_clock.RTC_sec >= 30 )){
            disable_pir_minutes = (my_clock.RTC_min + 1 + num_minutes) % 60;
          }else{
            disable_pir_minutes = (my_clock.RTC_min + num_minutes) % 60;
          }
          digitalWrite(pir_enable_pin, LOW);
          disable_pir_flag    = HIGH;
       } 
       
      default:
       break;
   }
   SPI_RX_index = 0;
   spi_interupt_flag = LOW;
}

void process_timer_expired(void){
  //now turn off relay
    P1OUT = ~P1OUT; //for debugging
   timer_interupt_flag = LOW;
  if(digitalRead(relay_set_pin) == HIGH){
      
       digitalWrite(relay_set_pin, LOW);
       digitalWrite(relay_reset_pin, HIGH);
  }
   timer_running_flag = LOW;
   end_timer_seconds   = 99;
   end_timer_minutes   = 99;
   end_timer_hours     = 99;
   if(disable_pir_flag == LOW){
     digitalWrite(pir_enable_pin, HIGH);
   }
}

/******************************
    Interupt Service Routines
******************************/

void motion_detected(void){
  motion_interupt_flag = HIGH;
}

//Using MSP430 native interupt notation
interrupt(TIMER1_A0_VECTOR) Tic_Tac(void) {
    my_clock++;            // Update time
    if( (my_clock.RTC_hr >= end_timer_hours) && 
          (my_clock.RTC_min >= end_timer_minutes) && 
            (my_clock.RTC_sec >= end_timer_seconds) ){
      timer_interupt_flag = HIGH;
       __bic_status_register_on_exit(LPM3_bits);
    }
   if((my_clock.RTC_min >= disable_pir_minutes) && 
            (my_clock.RTC_sec >= disable_pir_seconds) ){
      disable_pir_flag = LOW;
       digitalWrite(pir_enable_pin, HIGH);
       __bic_status_register_on_exit(LPM3_bits);
    }  
}

//MSP430 SPI RX Service Routine
interrupt(USCIAB0RX_VECTOR)  USCI0RX_ISR (void)
{
  if(IFG2 & UCB0RXIFG) {
    char value = UCB0RXBUF;
    if (value == '\n') { //ready to parse
        spi_interupt_flag = HIGH;
        __bic_status_register_on_exit(LPM3_bits);
    } else {
      SPI_RX_buff[SPI_RX_index] = value;
      SPI_RX_index++; 
    }  
  } 
}

/******************************
  Utility Functions
******************************/
void disable_wdt(void){
  WDTCTL = WDTPW + WDTHOLD;// Stop watchdog timer
}

/* Incase of Hardware failures */
void fault_routine(void) {
  P1OUT &= ~BIT0; //turn off powerlight for warning
  while(1); 
} 

/* Delay For Debug LED function.*/
void led_delay(uint32_t d) {
  int i;
  for (i = 0; i<d; i++) {
    nop();
  }
}
/* SPI Debug LED flash */
void flash_spi_detected(void) {
    int i=0;
    //P1OUT = ~P1OUT;
    for (i=0; i < 6; ++i) {
        //P1OUT = ~P1OUT;
        led_delay(0x4fff);
        led_delay(0x4fff);
    }
}

void setup_spi(void) {
  P1DIR |= BIT0;                       //For debug, remove for deployment

  UCB0CTL1 = UCSWRST;                  //Put msp430 USCI state machine in reset
  UCB0CTL0 = UCMODE_2|UCSYNC|UCCKPH;   //4-pin, 8-bit SPI slave
  UCB0CTL0 |= UCMSB;                   //Enable Most Singnificant bit first
  UCB0CTL0 &= ~UCMST;                  //Set to slave.
  P1SEL = BIT4 + BIT5 + BIT6 + BIT7;   //enable 4 SPI UCB0 pins
  P1SEL2 = BIT4 + BIT5 + BIT6 + BIT7; 
  UCB0CTL1 &= ~UCSWRST;                //Set USCI state machine**
  IFG2 &= ~UCB0RXIFG;                  //Clear pending recieve interupt
  IE2 |= UCB0RXIE;                     //Enable USCI0 RX interrupt
  
}

void set_sleep_timer(int seconds, int minutes, int hours) {
   end_timer_seconds = (my_clock.RTC_sec + seconds ) % 60;
   if( (end_timer_seconds <= 30 && ( seconds >= 30 || my_clock.RTC_sec >= 30 ) )
        || ( seconds >= 30 && my_clock.RTC_sec >= 30 )){
     end_timer_minutes = (my_clock.RTC_min + 1 + minutes) % 60;
   }else{
     end_timer_minutes = (my_clock.RTC_min + minutes) % 60;
   }
   if( (end_timer_minutes <= 30 && ( minutes >= 30 || my_clock.RTC_min >= 30 ) )
        || ( minutes >= 30 && my_clock.RTC_min >= 30 )){
     end_timer_hours = (my_clock.RTC_hr + 1 + hours) % 24;
   }else{
     end_timer_hours = (my_clock.RTC_hr + hours) % 24;
   }
}

void get_disable_time(void) {
   strncpy(rx_seconds, &SPI_RX_buff[3], 2);
   num_seconds = atoi(rx_seconds);
   strncpy(rx_minutes, &SPI_RX_buff[5], 2);
   num_minutes = atoi(rx_minutes);
}

void get_buffer_time(void) {
   strncpy(rx_seconds, &SPI_RX_buff[1], 2); 
   num_seconds = atoi(rx_seconds);
   strncpy(rx_minutes, &SPI_RX_buff[3], 2);
   num_minutes = atoi(rx_minutes);
   strncpy(rx_hours, &SPI_RX_buff[5], 2);
   num_hours = atoi(rx_hours);
}

void get_buffer_date(void) {
   strncpy(rx_year, &SPI_RX_buff[1], 4);
   num_year = atoi(rx_year);
   strncpy(rx_month, &SPI_RX_buff[5], 2);
   num_month = atoi(rx_month);
   strncpy(rx_day, &SPI_RX_buff[7], 2);
   num_day = atoi(rx_day);
}




