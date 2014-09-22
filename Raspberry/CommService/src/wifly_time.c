#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "wificmd.h"
// TODO check for failure, needs a way to make sure server is good, default
// servers fail. Using NIST server now, string parsing is hacky, default
// timezone is CA, need a way to change
int main() {
    // Open ports to the video server
    int fd;
    fd = initializeModule();
    char buffer[1024];
    
    cmdModeEnable(buffer, fd);
    serialPuts(fd, "\rset time address 128.138.140.44\r");
    serialReceive(buffer, fd);
    serialPuts(fd, "\rset time enable 1\r");
    serialReceive(buffer, fd);
    serialPuts(fd, "get time\r");
    serialReceive(buffer, fd);
    //printf(buffer);
    //printf("Check time:\n");
    serialPuts(fd, "show t t\r");
    delay(10);
    serialReceive(buffer, fd);

    // find beginning of unix time
    char *new = strstr(buffer, "RTC=");
    new = strchr(new, '=');
    new++;
    // finish at \r
    char *end = strchr(new, '\r');
    *end = 0;

    printf(new);

    serialClose(fd);
}
