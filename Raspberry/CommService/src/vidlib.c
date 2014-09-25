#include "vidlib.h"
#include "wificmd.h"

#include <wiringPi.h>
#include <wiringSerial.h>

#include <syslog.h>

// Note this code will leave the proc in VIDEO_DIRECTORY
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <dirent.h>

//TODO should not exit in here


#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

int fd;

void error(const char *msg)
{
    perror(msg);
}

// TODO check casts to uint32_t for overloading need checks for size limits
// Returns -1 on error
int send_video(int sockfd, char* filename) {
    int n;
    uint32_t id = (uint32_t) atoi(filename);
    //printf("id: %d, ", id);
    // First we need to get the size of the video, so we open the video file in
    // append mode, read the location (which will be the size of the file as
    // well) and rewind the file to the beginning for reading.
    FILE *fp = fopen(filename, "r");

    fseek(fp, 0L, SEEK_END);
    // TODO append, or use stat?
    // TODO size_t?
    uint32_t size = (uint32_t) ftell(fp);
    fseek(fp, 0L, SEEK_SET);




    //printf("size: %d\n", size);
    syslog(LOG_NOTICE, "sending video id: %d, size: %d", id, size);

    


    // Write ID to socket (sizeof uint32_t = 4)
    n = write(sockfd, &id, sizeof (uint32_t));

    if (n < 0) {
      //error("ERROR writing to socket");
      syslog(LOG_ERR, "ERROR writing to socket");
      return -1;
    }
    
    // Write ID to socket
    n = write(sockfd, &size, sizeof (uint32_t));
    if (n < 0) {
      //error("ERROR writing to socket");
      syslog(LOG_ERR, "ERROR writing to socket");
      return -1;
    }


    delay(10);
    // Write out file
    uint32_t rem = size;
    char buffer[1024];
    size_t r;
    int to_read;

    syslog(LOG_DEBUG, "writing video data");

    while (rem > 0) {
        to_read = min(rem, sizeof(buffer)); 
        //printf("To read: %d\n", to_read);
        r = fread(buffer, 1, to_read, fp);
        //printf("r: %d\n", r);

        if (r != to_read) {
            //error("ERROR file read error");
            syslog(LOG_ERR, "ERROR file read error");
            return -1;
        }
        rem -= r;

        n = write(sockfd, buffer, r);
        if (n < 0) {
            //error("ERROR writing to socket");
            syslog(LOG_ERR, "ERROR writing to socket");
            return -1;
        }
        delay(10);

    }    

    fclose(fp);
    
    syslog(LOG_INFO, "waiting for ACK");
    // ACK IT UP
    char id_s[256];
    sprintf(id_s, "%d\r", id);
    int l = strlen(id_s);
    int i = 0;
    int c;
    //int waits;
    while(1) {
        //waits = 0;
        // TODO recognize timeouts
        while (1) {
            c = serialGetchar(sockfd);
            if (c != -1) break;
            //waits++;
        }

       //printf("get %c\n", c);
       syslog(LOG_DEBUG, "get '%c', #%d in ack", c, c);
       if (c == id_s[i]) {
           i++;
       } else {
           delay(100);
           //serialReceive(buffer, sockfd);
           memset(buffer, 0, sizeof buffer);
           read(sockfd, buffer, sizeof buffer);
           //buffer[sizeof buffer - 1] = 0;
           //printf(buffer);
           syslog(LOG_DEBUG, "Wrong ACK, received: \"%s\"", buffer);
           
           //error("ERROR receiving ACK");
           syslog(LOG_ERR, "ERROR receiving ACK");
           return -1;
       }

       if (i == l) {
           break;
       }
    }
    if (remove(filename) == -1) {
        //error("ERROR deleting sent video file");    
        syslog(LOG_ERR, "ERROR deleting sent video file");    
        return -1;
    };

    return 0;

}

int send_video_directory(char* cam_name, char* dir_name, char* remote_host, char* remote_port) {
    syslog (LOG_INFO, "send_video_directory called in vidlib.c");

    // Open ports to the video server
    //int fd;
    fd = initializeModule();

    openConnection(fd,remote_host,remote_port);

    // Write camera name to socket, plus one to include the null byte
    //TODO puts + null byte to avoid strlen call
    int n = write(fd, cam_name, strlen(cam_name) + 1);
    if (n < 0) {
      //error("ERROR writing to socket");
      syslog(LOG_ERR, "ERROR writing to socket");
      goto ERROR;
    }

    chdir(dir_name);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            //printf ("%s, ", ent->d_name);
            if (send_video(fd, ent->d_name) == -1) {
                syslog(LOG_ERR, "error in send_video in vidlib");
                break;
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        //perror ("");
        syslog(LOG_ERR, "could not open video directory");
    }

ERROR:
    closeConnection(fd);
    serialClose(fd);
    return 0;

}
