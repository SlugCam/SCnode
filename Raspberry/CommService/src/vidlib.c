#include "vidlib.h"
#include "wificmd.h"

#include <wiringPi.h>
#include <wiringSerial.h>

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



#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// This function prints an error and quits. Used for fatal errors.
// From http://www.linuxhowtos.org/C_C++/socket.htm.
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// TODO check casts to uint32_t for overloading need checks for size limits
void send_video(int sockfd, char* filename) {
    int n;
    uint32_t id = (uint32_t) atoi(filename);
    printf("id: %d, ", id);
    // First we need to get the size of the video, so we open the video file in
    // append mode, read the location (which will be the size of the file as
    // well) and rewind the file to the beginning for reading.
    FILE *fp = fopen(filename, "r");

    fseek(fp, 0L, SEEK_END);
    // TODO append, or use stat?
    // TODO size_t?
    uint32_t size = (uint32_t) ftell(fp);
    fseek(fp, 0L, SEEK_SET);




    printf("size: %d\n", size);
    


    // Write ID to socket (sizeof uint32_t = 4)
    n = write(sockfd, &id, sizeof (uint32_t));
    if (n < 0) 
      error("ERROR writing to socket");
    
    // Write ID to socket
    n = write(sockfd, &size, sizeof (uint32_t));
    if (n < 0) 
      error("ERROR writing to socket");


    // Write out file
    uint32_t rem = size;
    char buffer[2048];
    size_t r;
    int to_read;

    while (rem > 0) {
        to_read = min(rem, sizeof(buffer)); 
        //printf("To read: %d\n", to_read);
        r = fread(buffer, 1, to_read, fp);
        //printf("r: %d\n", r);

        if (r != to_read) {
            error("ERROR file read error");
        }
        rem -= r;

        n = write(sockfd, buffer, r);
        if (n < 0) 
            error("ERROR writing to socket");

    }    

    fclose(fp);
    
    // ACK IT UP
    char id_s[256];
    sprintf(id_s, "%d\r", id);
    int l = strlen(id_s);
    int i = 0;
    char c;
    while(1) {
       c = serialGetchar(sockfd);
       printf("get %c\n", c);
       if (c == id_s[i]) {
           i++;
       } else {
           error("ERROR receiving ACK");
       }

       if (i == l) {
           break;
       }
    }
    if (remove(filename) == -1) {
        error("ERROR deleting sent video file");    
    };

}

int send_video_directory(char* cam_name, char* dir_name, char* remote_host, char* remote_port) {

    // Open ports to the video server
    int fd;
    fd = initializeModule();
    openConnection(fd,remote_host,remote_port);

    // Write camera name to socket, plus one to include the null byte
    //TODO puts + null byte to avoid strlen call
    int n = write(fd, cam_name, strlen(cam_name) + 1);
    if (n < 0) 
      error("ERROR writing to socket");

    chdir(dir_name);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            printf ("%s, ", ent->d_name);
            send_video(fd, ent->d_name);
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        return EXIT_FAILURE;
    }

    closeConnection(fd);
    serialClose(fd);
    return 0;

}
