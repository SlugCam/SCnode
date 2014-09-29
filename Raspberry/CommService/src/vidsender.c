#include <stdlib.h>
#include "vidlib.h"

int main(int argc, char** argv) {
   return send_video_directory(getenv("SC_CAMNAME"), "/slugcam/vid_out", getenv("SC_VSERVER"), getenv("SC_VPORT")); 
}
