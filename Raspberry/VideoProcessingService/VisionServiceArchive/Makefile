LIB_OBJS=RaspiCamControl.o RaspiPreview.o RaspiCLI.o
OBJS=mydemo.o mydemotagging.o $(LIB_OBJS)
BIN=mydemo.bin mydemotagging.bin

LDFLAGS+=-lvgfont -lfreetype -lz

BIN_DIR=/bin

CC=${BIN_DIR}/gcc
CXX=${BIN_DIR}/g++
LD=${BIN_DIR}/ld

CFLAGS= -Wall -I "/usr/include" \
              -I "/opt/vc/include" \
			  -I "/usr/include/wiringPi" \
			  -I "/opt/include/interface/vmcs_host/linux" \
			  -I "/opt/vc/src/hello_pi/libs/ilclient" \
			  -I "/opt/vc/src/hello_pi/libs/vgfont" \
			  -I "/opt/vc/include/interface/vcos" \
			  -I "/opt/vc/include/interface/vmcs_host/linux" \
			  -I "/opt/vc/include/interface/vcos/pthreads"

LDFLAGS=      -L "/opt/vc/lib" \
              -L "/usr/lib" \
              -L "/lib" \
              -L "/opt/vc/lib" \
              -L "/usr/lib" \
              -L "/lib" \
			  -L "/opt/vc/src/hello_pi/libs/ilclient" \
			  -L "/opt/vc/src/hello_pi/libs/vgfont" \
			  




CFLAGS+=-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi
# -lcairo -lopencv_nonfree -lopencv_gpu 
LDFLAGS+= -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lstdc++ -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab -lmmal_util -lmmal_core -lmmal_vc_client 

LDFLAGS+=-lvchostif -lm

all: $(BIN) $(LIB)

%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

%.o: %.cpp
	@rm -f $@ 
	$(CXX) $(CFLAGS) $(INCLUDES) -g -std=c++11 -c $< -o $@ -Wno-deprecated-declarations

%.bin: %.o $(LIB_OBJS)
	$(CC) -o $@ -Wl,--whole-archive $< $(LIB_OBJS) $(LDFLAGS) -Wl,--no-whole-archive -rdynamic

%.a: $(OBJS)
	$(AR) r $@ $^

clean:
	for i in $(OBJS); do (if test -e "$$i"; then ( rm $$i ); fi ); done
	@rm -f $(BIN) $(LIB)


