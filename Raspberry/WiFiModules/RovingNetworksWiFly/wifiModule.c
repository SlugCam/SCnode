#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "wificmd.h"

#include <wiringPi.h>
#include <wiringSerial.h>

#define RECEIVEBUFF_SIZE 1450


/*
 * Send Binary Data With video File byte stream.
 ******************************************************************************
*/
int sendVideo(){
	return 0;
}

/*
 * Create JSON messages.
 ******************************************************************************
*/
int makeJSON(char * type ){
	return 0;
}

/*
 * Parse JSON messages.
 ******************************************************************************
*/
int parseJSON(char * jsonString){
	return 0;
}

/*
 * Main
 ******************************************************************************
*/
int main (int argc, char **argv)
{
  int fd ;
  int count ;
  char response[RECEIVEBUFF_SIZE];

  // Sample JSON
  char sampleBatteryMsg[] ="\n{\n\"id\":42,\n\"cam\":\"soeQuad\",\n\"time\": 1405028345,\n\"type\":\"battery\",\n\"data\":{\n \"time\": 1405027345,\n \"batteryPercentage\": 70,\n \"timeRemaining\": 350,\n \"batteryCondition\": \"Good\"\n }, \n\"ack\":[]\n}\n"; 

  //open serial port
  if ((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

  //The wiringPi's kernel setup procedures
  if (wiringPiSetup () == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }

	fflush (stdout) ;
	serialFlush(fd);
 
	/*WiFi Module Enter Command mode
	if( cmdModeEnable(response, fd) ==1){
		printf("CMD mode Enabled\n");
	}else{
		printf("CMD mode Failed\n");
	}
	*/
  

    /*
    fflush (stdout) ;
  serialFlush(fd);
    delay (1000) ;
  //Open TCP connection
  char * tcpOpenCommand = "open 192.168.2.3 7892\r";
  serialPuts(fd,tcpOpenCommand);
  delay (600) ;
   while (serialDataAvail (fd) > 0)
    {
      response = serialGetchar (fd) ;
      fflush (stdout) ;
	  	printf ("Response: %c and the number %d \n", response, response) ;
    } 

  
  fflush (stdout) ;
  serialFlush(fd);

  connectWifi();
  serverConnect("localhost", "1234");
  makeJSON("Test");
  
  //Send JSON over serial
  //printf("%s\n\n",sampleBatteryMsg );
  serialPuts(fd,sampleBatteryMsg);
  delay (300) ;
   while (serialDataAvail (fd) > 0)
    {
      response = serialGetchar (fd) ;
      fflush (stdout) ;
	  	printf ("Response: %c and the number %d \n", response, response) ;
    } 

  //Recieve loop, recieve response from server over serial.


  parseJSON("JSON");

  fflush (stdout) ;
  serialFlush(fd);
  

  //WiFi Module Enter Command mode
  delay (300) ;
  serialPuts(fd,cmdCommand);
  delay (300) ;
   while (serialDataAvail (fd) > 0)
    {
      response = serialGetchar (fd) ;
      fflush (stdout) ;
	  	printf ("Response: %c and the number %d \n", response, response) ;
    }

	
  delay (300) ;
  char * closeCommand = " close\r";
  serialPuts(fd,closeCommand);
  delay (300) ;
   while (serialDataAvail (fd) > 0)
    {
      response = serialGetchar (fd) ;
      fflush (stdout) ;
	  	printf ("Response: %c and the number %d \n", response, response) ;
    }
	
  */

	//Exit command mode
    if( cmdModeDisable(response, fd) ==1){
		printf("CMD mode Disabled\n");
	}else{
		printf("Couldn't Exit CMD mode\n");
	}
	
	serialFlush(fd);
	serialClose(fd);
	return 0 ;
}
