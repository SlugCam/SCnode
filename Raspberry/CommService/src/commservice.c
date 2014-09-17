// Good references
// http://www.ibm.com/developerworks/library/l-ubuntu-inotify/

#include <stdio.h>

char cam_name[256];
char video_hostname[256];
char video_port[256];

int read_config(char* filename) {

}

int main(int argc, char** argv) {
    if (argc > 1) {
        read_config(argv[1]);
    } else {
        // read /etc/slugcam
    }

    // Send initial messages to begin receiving messages (within message thing)

    // Send priority messages
    
    // Send priority video
    
    // Send rest of messages
    
    // Send rest of video

    // Monitor video and message folders for new stuff

}
