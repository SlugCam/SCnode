/*
* Author: Kevin Abas
* Date: 3/5/2015
* Description: Header file for the MSP430's communication protocol with the 
*              Raspberry Pi inside the SlugCam system.
*/

#ifndef timerPacket_H
#define timerPacket_H

class TimerPacketClass{
	public:
	    int parseMessage(char* serialString);
	    unsigned int timeValue;
	  //private:
	    //private files here
};

extern TimerPacketClass TP;

#endif  
