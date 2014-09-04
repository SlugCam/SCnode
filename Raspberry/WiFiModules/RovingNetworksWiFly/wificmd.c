#include "wificmd.h"

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
		memset(&response[0], 0, sizeof(response));
		return 1;
	} else {
		memset(&response[0], 0, sizeof(response));
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
		memset(&response[0], 0, sizeof(response));
		serialPuts(serialLine,exitCommand);
		serialReceive(response, serialLine);
		if (checkCmdSyntax(response) == 0){
			memset(&response[0], 0, sizeof(response));
			return 1;
		}else{
			memset(&response[0], 0, sizeof(response));
			return 0;
		}
	}
	memset(&response[0], 0, sizeof(response));
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
int serverConnect(char* address, char* port){
	return 0;
}