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
    char response[RECEIVEBUFF_SIZE];

    fd = initializeModule();
    //connectWifi();
    openConnection(fd,"localhost", "1234");


    //makeJSON("Test");
    //Send JSON over serial
    //printf("%s\n\n",sampleBatteryMsg );
    serialPuts(fd,"TEST");
    delay (300) ;
    serialReceive(response, fd);
    printf(response);

    closeConnection(fd);
    serialClose(fd);
    return 0 ;
}
