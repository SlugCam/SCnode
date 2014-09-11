#include "wificmd.h"


#include <wiringPi.h>
#include <wiringSerial.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define RECEIVEBUFF_SIZE 1450

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


    fd = initializeModule();
    //connectWifi();
    openConnection(fd,"localhost", "1234");


    //makeJSON("Test");
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


    //parseJSON("JSON");

    fflush (stdout) ;
    serialFlush(fd);

    //WiFi Module Enter Command mode
    closeConnection(fd);
    serialFlush(fd);
    serialClose(fd);
    return 0 ;
}
