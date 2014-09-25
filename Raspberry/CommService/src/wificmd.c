#include "wificmd.h"

#include <syslog.h>
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
        //fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        syslog (LOG_ERR, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1 ;
    }

    //The wiringPi's kernel setup procedures
    if (wiringPiSetup () == -1)
    {
        //fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        syslog (LOG_ERR, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }

    fflush (stdout) ;
    //serialFlush(fd);

    return fd;

}

/*
 * Return recieved string from serial line. Function returns number of characters recieved.
 ******************************************************************************
 */
int serialReceive(char * response, int serialLine ){
    int i = 0;
    delay(300);

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
    serialFlush(serialLine);
    delay (300) ;
    serialPuts(serialLine,"$$$");
    delay (300) ;

    int size = serialReceive(response, serialLine);
    if (size == 0){
        //printf("Nothing read as response\n");
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

    serialReceive(response, serialLine);

    //printf("cmdModeDisable response: %s\n", response);
    syslog(LOG_DEBUG, "cmdModeDisable response: %s\n", response);

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
    //printf("openConnection called to %s:%s\n", address, port);
    syslog(LOG_DEBUG, "openConnection called to %s:%s\n", address, port);


    /*WiFi Module Enter Command mode */
    if( cmdModeEnable(response, fd) ==1){
        //printf("CMD mode Enabled\n");
        syslog(LOG_DEBUG, "CMD mode Enabled\n");
    }else{
        //printf("CMD mode Failed\n");
        syslog(LOG_DEBUG, "CMD mode Failed\n");
    }

    serialPuts(fd,"close\r");
    serialReceive(response, fd);
    //printf("response to 'close': %s\n", response);
    syslog(LOG_DEBUG,"response to 'close': %s\n", response);

    serialPuts(fd,"set comm remote 0\r");
    serialReceive(response, fd);
    //printf("response to 'set comm remote 0': %s\n", response);
    syslog(LOG_DEBUG,"response to 'set comm remote 0': %s\n", response);

    //fflush (stdout) ;
    //serialFlush(fd);
    //delay (1000) ;

    //Open TCP connection
    // Command in form: "open "192.168.2.3 7892\r"
    //char * tcpOpenCommand = "open "192.168.2.3 7892\r";
    
    serialPrintf(fd, "open %s %s\r", address, port);
    serialReceive(response, fd);    
    //printf("response to open command: %s\n", response);
    syslog(LOG_DEBUG,"response to open command: %s\n", response);

    //fflush (stdout) ;
    //serialFlush(fd);

    //cmdModeDisable(response, fd);
    //printf("response to exit command: %s\n", response);
    return 0;
}

int closeConnection (int fd) {
    //printf("closeConnection called\n");
    syslog(LOG_DEBUG,"closeConnection called\n");

    cmdModeEnable(response, fd);

    //delay (300) ;
    char * closeCommand = "close\r";
    serialPuts(fd,closeCommand);
    //delay (300) ;

    serialReceive(response, fd);
    //printf("response to close command: %s\n", response);
    syslog(LOG_DEBUG,"response to close command: %s\n", response);


    //Exit command mode
    if( cmdModeDisable(response, fd) ==1){
        //printf("CMD mode Disabled\n");
        syslog(LOG_DEBUG,"CMD mode Disabled\n");
    }else{
        //printf("Couldn't Exit CMD mode\n");
        syslog(LOG_DEBUG,"Couldn't Exit CMD mode\n");
    }
    serialReceive(response, fd);
    //printf("response to exit command: %s\n", response);
    syslog(LOG_DEBUG,"response to exit command: %s\n", response);
    return 0;

}
