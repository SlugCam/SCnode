#include <wiringSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int serialReceive(char * response, int serialLine );

int checkCmdSyntax(char * response);

int cmdModeEnable(char * response, int serialLine);

int cmdModeDisable(char * response, int serialLine);

int connectWifi();

int serverConnect(char* address, char* port);

