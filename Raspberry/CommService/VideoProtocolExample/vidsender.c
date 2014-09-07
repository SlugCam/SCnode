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

#define CAMERA_NAME "c1"
#define VIDEOSERVER_PORT 7893
#define VIDEOSERVER_HOSTNAME "localhost"
#define VIDEO_DIRECTORY "./vids/"


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

// This simple function opens a port and returns the file descriptor for the
// port. Alot from http://www.linuxhowtos.org/C_C++/socket.htm.
int create_socket(int portno, char* hostname) {
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) 
    error("ERROR opening socket");

  server = gethostbyname(hostname);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    error("ERROR connecting");
  
  return sockfd;
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
    

    // Write camera name to socket, plus one to include the null byte
    n = write(sockfd, CAMERA_NAME, strlen(CAMERA_NAME) + 1);
    if (n < 0) 
      error("ERROR writing to socket");

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
    //strcpy(buffer, CAMERA_NAME);
    //n = write(sockfd,buffer,strlen(buffer));
    //if (n < 0) 
    //  error("ERROR writing to socket");
}


int main() {
    // Open ports to the video server
    int sockfd = create_socket(VIDEOSERVER_PORT, VIDEOSERVER_HOSTNAME);


    chdir(VIDEO_DIRECTORY);
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            printf ("%s, ", ent->d_name);
            send_video(sockfd, ent->d_name);
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        return EXIT_FAILURE;
    }

    /*
    // Create a buffer to hold path names
    DIR *dir;
    struct dirent *ent;

    char fullpath[strlen(VIDEO_DIRECTORY) + NAME_MAX + 1];

    if ((dir = opendir(VIDEO_DIRECTORY)) != NULL) {
        /* print all the files and directories within directory * /
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            printf ("%s = ", ent->d_name);
            sprintf(fullpath, "%s%s", VIDEO_DIRECTORY, ent->d_name);
            printf ("%s\n", fullpath);
        }
        closedir (dir);
    } else {
        /* could not open directory * /
        perror ("");
        return EXIT_FAILURE;
    }
    */


    //send_video(sockfd, 1, "1.avi");

    close(sockfd);


    /*
       n = write(sockfd,buffer,strlen(buffer));
       if (n < 0) 
       error("ERROR writing to socket");
       bzero(buffer,256);
       n = read(sockfd,buffer,255);
       if (n < 0) 
       error("ERROR reading from socket");
       printf("%s\n",buffer);
       */

    return 0;
}
