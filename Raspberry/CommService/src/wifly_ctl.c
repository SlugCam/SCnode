#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "wificmd.h"

// TODO check for failure, needs a way to make sure server is good, default
// servers fail. Using NIST server now, string parsing is hacky, default
// timezone is CA, need a way to change
//
int main(int argc, char** argv) {



    if (argc != 2) {
        printf("Usage: %s COMMAND\nCommands: sleep, wake\n", argv[0]);
    }
    // Open ports to the video server
    int fd;
    fd = initializeModule();
    char buffer[1024];


    if (strcmp(argv[1], "sleep") == 0) { 
        cmdModeEnable(buffer, fd);
        serialPuts(fd, "\rsleep\r");
        printf("Wifly sleeping...\n");
    } else if (strcmp(argv[1], "wake") == 0) { 
        serialPuts(fd, "a");
        delay(4000);
        serialReceive(buffer, fd);
        printf(buffer);
    } else if (strcmp(argv[1], "ping") == 0) { 
        openConnection(fd,getenv("SC_MSERVER"),getenv("SC_MPORT"));
        serialPrintf(fd, "{ \"id\":0, \"cam\":\"%s\",\"time\":%d,\"data\":{},\"type\":\"ping\"}\n", getenv("SC_CAMNAME"), time(NULL));
        /*
        id: 0,
        cam: this.name,
        time: Math.floor((new Date()).getTime() / 1000),
        type: 'ping',
        data: {}
        */
        delay(1000);
        serialReceive(buffer, fd);
        printf(buffer);
        closeConnection(fd);
    } else if (strcmp(argv[1], "tag") == 0) { 
        openConnection(fd,getenv("SC_MSERVER"),getenv("SC_MPORT"));
        serialPrintf(fd, "{ \"id\":0, \"cam\":\"%s\",\"time\":%d,\"data\":{\"message\":\"Human!\"},\"type\":\"tag\"}\n", getenv("SC_CAMNAME"), time(NULL));
        /*
        id: 0,
        cam: this.name,
        time: Math.floor((new Date()).getTime() / 1000),
        type: 'ping',
        data: {}
        */
        delay(1000);
        serialReceive(buffer, fd);
        printf(buffer);
        closeConnection(fd);
    } else { 
        printf("ERROR: invalid command\n");
    }
    serialClose(fd);
    return 0;
}
