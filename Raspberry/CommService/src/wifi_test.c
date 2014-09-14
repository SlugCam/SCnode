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
    openConnection(fd,argv[1],argv[2]);


    serialPuts(fd,"TEST");
    delay (300) ;
    serialReceive(response, fd);
    printf("response to test message: %s\n", response);

    closeConnection(fd);
    serialClose(fd);
    return 0 ;
}
