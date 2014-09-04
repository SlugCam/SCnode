/*
* debugUART.c
* Author: Kevin Abas
* Description: This file is used to send commands to the WiFi module
* and then display its output to the terminal.
*/

#include <wiringSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#define BAUD 115200

int main(int argc, char **argv){
	

	int fd = serialOpen("/dev/ttyAMA0", BAUD) ;

	char *command = argv[1];
	printf("%s",command);

	if(command[0] != '$'){
		int len = strlen(command);
		command[len]= '\r';
	  	command[len+1] = '\0';
	}
	serialPuts(fd, command);

	char response[1000];
	do{
	  	int len = strlen(response);
	  	response[len]=serialGetchar(fd);
	  	response[len+1] = '\0';
	 }while(serialDataAvail(fd)>0);

	printf("%s",response);

	serialFlush(fd);
	serialClose(fd);

	return 0;
}