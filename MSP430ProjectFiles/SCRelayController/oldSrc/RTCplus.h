/*
 RTCplus.h - Library for RTC capabilities using MSP430 hardware
 
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
 v1.01 - 2013-01-11 added #include <legacyMSP430.h> to this header
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
 
4 Call begin() in your Setup() section.
 
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
 
// ensure this library description is only included once
#ifndef RTCplus_h
#define RTCplus_h

#include <legacyMSP430.h>

//Uncomment the following #DEFINE to enable Date support
#define RTCWITHDATE
//Uncomment the following #DEFINE to enable 1/256 ticks
//#define RTCSUBSECONDS

// library interface description
class RealTimeClock
{
    // user-accessible "public" interface
public:
    RealTimeClock(void);
    void begin(void);                            //sets up the clocks.
    void Set_Time(char hr, char mins, char secs);
    void Set_Date(int year, char month, char day);
    void Set_Year(int year);
#ifdef RTCSUBSECONDS
    int RTC_chunk; // this holds the number of 1/256 time samples.
#endif
    char RTC_sec; // This how you read the time, by reading the vars
    char RTC_min;
    char RTC_hr;
#ifdef RTCWITHDATE
    char RTC_day;
    char RTC_month;
    int  RTC_year;
#endif
    RealTimeClock& operator++();    // Overload ++ operator for writing convenience (Prefix Variant)
    RealTimeClock& operator++(int); // PostFix variant
 
    void Inc_sec(void);  //the Inc_ functions allow you to increment one value, but if
    void Inc_min(void);  //it overflows, it will then carry on up the line, eg if time
    void Inc_hr(void);   //is 23:59:59 and you call Inc_sec, it will overflow to 0, then call Inc_min making it 0, then call Inc_hour, making it 0, which would then call Inc_day (if RTCWITHDATE is defined).
#ifdef RTCWITHDATE
    void Inc_day(void);
    void Inc_month(void);
    void Inc_year(void);
#endif 

#ifdef RTCSUBSECONDS
private:
    void Inc_chunk(void); // increment the chunk counter.
#endif

};
 
#endif