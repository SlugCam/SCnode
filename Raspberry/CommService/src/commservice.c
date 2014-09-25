// Good references
// http://www.ibm.com/developerworks/library/l-ubuntu-inotify/

#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

// For directory checking
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "vidlib.h"


char cam_name[256];
char video_hostname[256];
char video_port[256];
char video_dir[256];

/*
int dir_isempty(const char* dirname) {
    int ret;
    DIR* dir = opendir(dirname);
    // should throw error
    if (dir == NULL) return 1;
    ret = readdir(dir) == NULL;
    closedir(dir);
    return ret;
}*/

// Returns 1 is dir has at least one file
int dir_hasfile(const char* dirname)
{
    int ret=0;
    struct dirent* ent;
    DIR* dir = opendir(dirname);

    // TODO should throw error
    if (dir == NULL) return 0;

    while((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        } else {
            ret = 1;
            break;
        }
    }
    closedir(dir);

    return ret;
}

int read_config(char* filename) {

    return 0;
}

int main(int argc, char** argv) {
    /*
    if (argc > 1) {
        read_config(argv[1]);
    } else {
        // read /etc/slugcam
    }
    */



    //setlogmask (LOG_UPTO (LOG_DEBUG));

    openlog ("commservice", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    syslog (LOG_NOTICE, "Program started by User %d", getuid());
    //syslog (LOG_INFO, "A tree falls in a forest");//GNU doc example





    char *cam_name = "SlugCam1",
         *video_hostname = "skynet.soe.ucsc.edu",
         *video_port = "7893",
         *video_dir = "/slugcam/vid_out";

    // Send initial messages to begin receiving messages (within message thing)

    // Send priority messages
    

    // Send priority video
    //
    //
    // Should eventually use inotify instead of polling
    while (1) {

        //printf("checking %s...\n", video_dir);
        syslog (LOG_DEBUG, "checking %s...", video_dir);

        if (dir_hasfile(video_dir)) {
            send_video_directory(cam_name, video_dir, video_hostname, video_port);
        } else {
            //printf("empty\n");
            syslog (LOG_DEBUG, "empty");
        }
        sleep(2);
    }
    
    // Send rest of messages
    
    // Send rest of video

    // Monitor video and message folders for new stuff
    closelog ();
    return 0;

}
