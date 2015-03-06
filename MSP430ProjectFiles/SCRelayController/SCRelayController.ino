/*
 Slugcam MSP430 Firmware
 
 Date: 2/22/2015
 Author: Kevin Abas
 Last Modified: 2/28/2015
 
 This program allows the MSP430 the ability to monitor the passive infared sensor for motion, and
 to power on the SlugCam system using a dual coil relay. SlugCam also uses the MSP430 to then
 keep the SlugCam system awake by using an MSP430 on board timer. SlugCam communicates how long
 it wishes to stay awake using serial SPI communication. Should motion be a nusence, SlugCam
 also has the ability to tell the MSP430 to ignore the motion interupts for a longer period of time.
 
 Circuit:
   PIR Sensor:
     DOUT: pin 11 (P2.3
     EN: pin 12 (P2.4)
   SPI:
     MOSI: pin 15 (P1.7)
     MISO: pin 14 (P1.6)
     CE0: pin 6 (P1.4)
     SCLK: pin 7 (P1.5)
     
   Relay:
     Set: pin 9 (P2.1)
     Reset: pin 10 (P2.2)
 */


volatile int motionInteruptFlag = LOW;
volatile int spiInteruptFlag = LOW;
volatile int timerInteruptFlag = LOW;

const int spiChipEnablePin = 6;
const int relaySetPin = 9;
const int relayResetPin = 10;
const int pirDoutPin = 11; 
const int pirEnablePin = 12;


/******************************
    Configuration Function
******************************/
void setup() {

  // initialize the pins for I/O
  pinMode(spiChipEnablePin, INPUT);
  pinMode(relaySetPin, OUTPUT);
  pinMode(relayResetPin, OUTPUT);
  pinMode(pirDoutPin, INPUT);
  pinMode(pirEnablePin, OUTPUT);

  attachInterrupt(pirDoutPin, motionDetected, RISING);
  attachInterrupt(spiChipEnablePin, spiCommReady, RISING);

  digitalWrite(relaySetPin, LOW);

  disableWDT(); // Watch Dog Timer
  configureACLK(); //sets up onboard 12KHz clock
  configureTimer(60); //setup Timer_A with starting coundown(seconds)
  
  //_BIS_SR(GIE);
  
  
  // give SlugCam time to boot
  //delay(1500);
}


/******************************
        Main Loop
******************************/
void loop() {
  
  
  if(motionInteruptFlag == HIGH){
    processMotion();
  }else if(spiInteruptFlag == HIGH){
    processSPI();
  }else if(timerInteruptFlag == HIGH){
    processTimerExpired();
  }else if((motionInteruptFlag == LOW) && (spiInteruptFlag == LOW) &&
                (timerInteruptFlag == LOW)){
    //deep sleep  
  }
  
}

/******************************
  Interupt Handling Functions
******************************/
void processMotion(){
  //If relay isn't on turn it on
  if(digitalRead(relaySetPin) == LOW){
       digitalWrite(relaySetPin, HIGH);
       digitalWrite(relayResetPin, LOW);
  }
  motionInteruptFlag = LOW;  
  
}

void processSPI(void){
    //add SPI I/O  
}

void processTimerExpired(void){
  //delay power off a few 3 more seconds
  
  
  //now turn off relay
  if(digitalRead(relaySetPin) == HIGH){
       digitalWrite(relaySetPin, LOW);
       digitalWrite(relayResetPin, HIGH);
  }
 timerInteruptFlag = LOW; 
}


/******************************
    Interupt Service Routines
******************************/

void motionDetected(void){
  motionInteruptFlag = HIGH;
}

void spiCommReady(void){
  spiInteruptFlag = HIGH;
  
}

//Using MSP430 native interupt notation
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  timerInteruptFlag = HIGH;
  CCR0 += 12000 * 60;
} 

/******************************
  MSP430 Functions
******************************/
void disableWDT(void){
  WDTCTL = WDTPW + WDTHOLD;
}

void configureACLK(void) {
// Stop watchdog timer
  if (CALBC1_1MHZ == 0xFF || CALDCO_1MHZ == 0xFF)
    //FaultRoutine();// If clock calibration data has been erased
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  BCSCTL3 |= LFXT1S_2;
  IFG1 &= ~OFIFG;
  BCSCTL2 |= SELM_0 + DIVM_3 + DIVS_3;
}

//Incase of Hardware failures or security breach
void FaultRoutine(void) {
  P1OUT &= ~BIT0; //turn off powerlight for warning
  while(1); 
} 

void configureTimer(int startSeconds){
  CCTL0 = CCIE;
  CCR0 = 12000 * startSeconds;
  TACTL = TASSEL_1 + MC_2;
}

