// Good references
// http://www.ibm.com/developerworks/library/l-ubuntu-inotify/

#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>

// For directory checking
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "vidlib.h"

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


int main(int argc, char** argv) {
    //setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog ("commservice", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (LOG_NOTICE, "Program started by User %d", getuid());

    char *cam_name = getenv("SC_CAMNAME"),
         *video_hostname =  getenv("SC_VSERVER"),
         *video_port =  getenv("SC_VPORT"),
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
