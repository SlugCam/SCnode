/*
 RTCplus.cpp - Library for RTC capabilities using MSP430 hardware
 
 The bulk of this library is copied from sRTC v1.03 by Yannick DEVOS (XV4Y)
 This library has the following modifications made by Graham Fountain:
 * Setup of clocks is done in the begin() routine to allow correct operation on Energia 0009
 * Optional date recording in addition to time
 * Basic leap year functionality. It will recognise Feb has 29 days in years that are a multiple of 4.
   However, it doesn't allow for the fact that century years are not leap years unless multiples of 400.
 * Two operation modes - counting in 1/256sec intervals if time measurement less than 1 second is needed
   or counting in 1sec intervals. 1sec counting should allow lower power consumption and has smaller code.
 The library is compatible with sRTC, so existing programs should operate simply by changing the header.

 Any commercial use or inclusion in a kit is subject to author approval
 
 ====
 Revision history :
 v1.00 - 2013-01-08 initial release of RTCplus.h
 ====
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You can download a copy of the GNU General Public License at <http://www.gnu.org/licenses/>
 
///////////////////////////////// How to use this library

1 This library is designed to operate on the MSP430 launchpad with a 32.768kHz crystal connected to
  pins 2_6 & 2_7. 
 
2 Enable date features or sub-second measurement if you need them, by editing the RTCplus.h file and
  removing the '//' from the lines with the #defines.
  #define RTCWITHDATE enables the use of the day, month & year values.
  #define RTCSUBSECONDS enables 1/256second ticks for more precision. without this it is 1 second ticks.
  Each feature that is enabled adds to code size, so it is recommended that you only turn on what you need.
  
3 Create an instance of the object by including the following code in your sketch
 
    RealTimeClock myClock;
 
4 Do low-level Timer and Clock setup yourself or call begin() in your Setup() section.
 
    myClock.begin();
 
5 Create an interrupt routine to be called when the timer triggers. This interrupt routine must call the
  Real-Time-Clock increment function. It can then do other functions if needed. You should add this code 
  to the end of your main program :
 
    interrupt(TIMER1_A0_VECTOR) Tic_Tac(void) {
        myClock.++;            // Update chunks
    };
 
6 - To access the time read RTC_sec, RTC_min, RTC_hr variables eg myClock.RTC_sec
  - If you enabled sub-second counting, you can also get the number of 1/256 second ticks with RTC_chunk
  - If you enabled dates, you can read RTC_day, RTC_month & RTC_year
  - Time variables are 0 based, ie 0-23 hrs, 0-59 minutes, 0-59 seconds, 0-255 ticks
  - Date variables are 1 based, ie 1-31 days, 1-12 months
  - The default initial values are 0:0:0, 2013-1-1 (midnight of the morning of 1 January, 2013)

7 - To set the time, call Set_Time(hours, minutes, seconds)
  - If you enabled dates, you can also call Set_Date(year, month, day) & Set_Year(year)
  - Set_Time and Set_Date perform silent bounds checking - if the month, day, hours, minutes or seconds are
    out of bounds they will not alter the values. However Set_Date does ALWAYS alter the year.
  - Increment functions are also available Inc_sec(), Inc_min(), Inc_hr(), Inc_day(), Inc_month(), Inc_year()
  - The increment functions increment with carry, so if it is 23:59:59 2013-12-31 and you call Inc_sec(), it
    will roll over to 0:0:0 2014-1-1. The original sRTC had these as private functions, but I have made them
    public as it is envisaged that you might call these routines as part of a user interface to set time.
  - It is possible, but not recommended, to directly alter RTC_sec, RTC_min etc. If you directly alter
    these variables, you will need to be responsible for bounds checking. If RTC_year is directly altered
    it may not correctly recognise leap years, so Set_Year should always be called instead.

8 - you can also create conditional compile sections within your own code through use of RTCWITHDATE &
    RTCSUBSECONDS 
 
 */
 
// include this library's description file
#include <RTCplus.h>
#include <legacymsp430.h>
 
// Constructor /////////////////////////////////////////////////////////////////
 
RealTimeClock::RealTimeClock(void)
{
    //initialise our counters all to 0
#ifdef RTCSUBSECONDS
    RTC_chunk = 0;
#endif
    RTC_sec = 0;
    RTC_min = 0;
    RTC_hr = 0; 
#ifdef RTCWITHDATE
    RTC_day = 1;
    RTC_month = 1;
    RTC_year = 2013;
#endif

};
 
// Methods /////////////////////////////////////////////////////////////////
 
void RealTimeClock::begin(void) { //Set up our clock
     
 
    BCSCTL1 &= ~DIVA_3;     // ACLK without divider, set it to DIVA_0

    P2SEL |= (BIT6 | BIT7); // This is to override default Energia settings that set P2_6 and P2_7 to GPIO instead of ACLK
    BCSCTL3 = (LFXT1S_0 | XCAP_3);// Override default Energia setting sourcing ACLK from VLO, now source from XTal

     
    TA1CCTL0 = CCIE;             //  CCR0 interupt activated
#ifdef RTCSUBSECONDS
    TA1CCR0 = 128-1;             // 128 ticks is 1/256th of a second
    TA1CTL = TASSEL_1 | ID_0 | MC_1;  // Clock for TIMER 1 = ACLK, No division, up mode
#else
    TA1CCR0=4096-1;                     // we are dividing clock by 8, so 4096 ticks = 1 second
    TA1CTL = TASSEL_1 | ID_3 | MC_1;	// Clock for TIMER 1 = ACLK, By 8 division, up mode
#endif
 
     
};
 
RealTimeClock& RealTimeClock::operator++() {    // Overload ++ operator for writing convenience (Prefix Variant)
#ifdef RTCSUBSECONDS
    Inc_chunk();    //  we have sub-seconds enabled, so call Inc_chunk
#else
    Inc_sec();      // or if we have 1 second resolution, increment our seconds.
#endif
    return *this;
};
 
RealTimeClock& RealTimeClock::operator++(int) { // PostFix variant
    RealTimeClock tmp(*this);
    ++(*this);
    return tmp;
};

#ifdef RTCSUBSECONDS 
void RealTimeClock::Inc_chunk(void) {
    if (++RTC_chunk >= 256) {
        RTC_chunk=0;
        Inc_sec();
    };
};
#endif
 
void RealTimeClock::Inc_sec(void) {
    if (++RTC_sec >= 60) {
        RTC_sec=0;
        Inc_min();
    };
};
 
void RealTimeClock::Inc_min(void) {
    if (++RTC_min >= 60) {
        RTC_min=0;
        Inc_hr();
    };
};
 
void RealTimeClock::Inc_hr(void) {
    if (++RTC_hr >= 24) {
        RTC_hr=0;
#ifdef RTCWITHDATE
        Inc_day(); //if we have dates, we need to step up our day as well.
#endif
    };
};
 
void RealTimeClock::Set_Time(char hrs=0, char mins=0, char secs=0) {
    if ((hrs>=0 && hrs <24) && (mins>=0 && mins<60) && (secs>=0 && secs<60)) {
        RTC_hr = hrs;   // Set time to values given in parameters
        RTC_min = mins;
        RTC_sec = secs;
#ifdef RTCSUBSECONDS
        RTC_chunk = 0;
#endif
    };
};


//from here on, are our commands to do with dates.
#ifdef RTCWITHDATE
//array of Days per month. At the expense of 1 extra byte, we avoid having to do additions/subtractions to cope
//with converting our 1..12 months to 0..11 array indices. easier to waste 0, and lookup 1..12
char RTC_DaysPerMonth[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};

void RealTimeClock::Set_Date(int year=2013, char month=1, char day=1) {
    Set_Year(year); //set the year regardless - it will make the next check possible.
    if ((month>=1 && month<=12) && (day>=1 && day<=RTC_DaysPerMonth[month])){ 
      RTC_day=day;
      RTC_month=month;
    };
};

void RealTimeClock::Inc_day(void) {
  if (++RTC_day>RTC_DaysPerMonth[RTC_month]) {
    RTC_day=1;
    Inc_month();
  };

}; 

void RealTimeClock::Inc_month(void) {
  if (++RTC_month>12) {
    RTC_month=1;
    Inc_year();
  };
};

void RealTimeClock::Set_Year(int year) {
  RTC_year=year;
  RTC_DaysPerMonth[2]=(RTC_year&0x3)?28:29;  //set the number of days for February in our array
};

void RealTimeClock::Inc_year(void) {
  Set_Year(RTC_year+1);
};
#endif