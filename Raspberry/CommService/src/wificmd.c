#include "wificmd.h"

#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// TODO deal with responses in a more rational way, suggestion use buffer for
// all responses, and when error, this buffer can be checked.

char response[1024];

// Initialize the WiFi Module serial port and return the file descriptor for the
// serial port.
int initializeModule() {
    int fd;

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

    return fd;

}


/*
 * Return recieved string from serial line. Function returns number of characters recieved.
 ******************************************************************************
 */
int serialReceive(char * response, int serialLine ){
    int i = 0;

    while ( serialDataAvail (serialLine) > 0 )
    {
        response[i] = serialGetchar (serialLine) ;
        i++;
    }
    response[i] = 0;

    return i;
}


/*
 * Check if command sent wasn't interpreted by module.
 ******************************************************************************
 */
int checkCmdSyntax( char * response){

    if( strcmp(response, "\nERR: ?-Cmd\n<4.00>\n") == 0){
        return 1;
    } else {
        return 0;
    }

}

/*
 * Put the Wifi Module into command mode.
 ******************************************************************************
 */
int cmdModeEnable(char * response, int serialLine){
    char *cmdCommand = "$$$";
    serialPuts(serialLine,cmdCommand);
    delay (300) ;

    int size = serialReceive(response, serialLine);
    if (size == 0){
        printf("Nothing read as response\n");
        return 0;
    }else if( strcmp(response, "CMD\r\n") == 0){
        return 1;
    } else {
        return 0;
    }
}

/*
 * Tell the Wifi module to exit command mode
 ******************************************************************************
 */
int cmdModeDisable(char * response, int serialLine){
    char * exitCommand = "exit\r";
    serialPuts(serialLine,exitCommand);

    int size = serialReceive(response, serialLine);
    int i = 0;
    while (i< size){
        printf("%d\n", response[i] );
        i++;
    }
    if (checkCmdSyntax(response) == 1){
        serialPuts(serialLine,exitCommand);
        serialReceive(response, serialLine);
        if (checkCmdSyntax(response) == 0){
            return 1;
        }else{
            return 0;
        }
    }
    return 1;
}


/*
 * Associate to Wifi AP if nessecary.
 ******************************************************************************
 */
int connectWifi(){
    return 0;
}

/*
 * Function to open a TCP connection with remote webserver.
 ******************************************************************************
 */
int openConnection(int fd, char* address, char* port){

    /*WiFi Module Enter Command mode */
    if( cmdModeEnable(response, fd) ==1){
        printf("CMD mode Enabled\n");
    }else{
        printf("CMD mode Failed\n");
    }

    serialPuts(fd,"set comm remote 0");

    fflush (stdout) ;
    serialFlush(fd);
    delay (1000) ;

    //Open TCP connection
    // Command in form: "open "192.168.2.3 7892\r"
    //char * tcpOpenCommand = "open "192.168.2.3 7892\r";
    serialPrintf(fd, "open %s %s\n", address, port);

    serialReceive(response, fd);    
    printf("%s\n", response);

    fflush (stdout) ;
    serialFlush(fd);

    cmdModeDisable(response, fd);
    return 0;
}

int closeConnection (int fd) {
    cmdModeEnable(response, fd);

    delay (300) ;
    char * closeCommand = "close\r";
    serialPuts(fd,closeCommand);
    delay (300) ;

    serialReceive(response, fd);


    //Exit command mode
    if( cmdModeDisable(response, fd) ==1){
        printf("CMD mode Disabled\n");
    }else{
        printf("Couldn't Exit CMD mode\n");
    }

}
