TOOLCHAIN_DIR=../SlugcamBuildrootImage/toolchain

BIN_DIR=${TOOLCHAIN_DIR}/host/usr/bin

CC=${BIN_DIR}/arm-buildroot-linux-uclibcgnueabi-gcc
CXX=${BIN_DIR}/arm-buildroot-linux-uclibcgnueabi-g++
LD=${BIN_DIR}/arm-buildroot-linux-uclibcgnueabi-ld

BUILDROOT_STAGING_DIR=${TOOLCHAIN_DIR}/staging
BUILDROOT_TARGET_DIR=${TOOLCHAIN_DIR}/target


CFLAGS= -Wall -I "$(BUILDROOT_STAGING_DIR)/usr/include" \
              -I "$(BUILDROOT_STAGING_DIR)/opt/vc/include" \
			  -I "$(BUILDROOT_STAGING_DIR)/usr/include/wiringPi" \
              -L "$(BUILDROOT_STAGING_DIR)/opt/vc/lib" \
              -L "$(BUILDROOT_STAGING_DIR)/usr/lib" \
              -L "$(BUILDROOT_STAGING_DIR)/lib" \
              -L "$(BUILDROOT_TARGET_DIR)/opt/vc/lib" \
              -L "$(BUILDROOT_TARGET_DIR)/usr/lib" \
              -L "$(BUILDROOT_TARGET_DIR)/lib" 

all: directories bin/wifi_test bin/vidsender bin/commservice bin/wifly_time bin/wifly_ctl

bin/wifi_test: build/wifi_test.o build/wificmd.o
	${CC} ${CFLAGS} build/wifi_test.o build/wificmd.o -lwiringPi -lpthread -o bin/wifi_test

bin/vidsender: build/vidsender.o build/wificmd.o build/vidlib.o
	${CC} ${CFLAGS} build/vidsender.o build/vidlib.o build/wificmd.o -lwiringPi -lpthread -o bin/vidsender

bin/commservice: build/commservice.o build/wificmd.o build/vidlib.o
	${CC} ${CFLAGS} build/commservice.o build/vidlib.o build/wificmd.o -lwiringPi -lpthread -o bin/commservice

bin/wifly_time: build/wifly_time.o build/wificmd.o
	${CC} ${CFLAGS} build/wifly_time.o build/wificmd.o -lwiringPi -lpthread -o bin/wifly_time

bin/wifly_ctl: build/wifly_ctl.o build/wificmd.o
	${CC} ${CFLAGS} build/wifly_ctl.o build/wificmd.o -lwiringPi -lpthread -o bin/wifly_ctl

build/%.o: src/%.c 
	$(CC) $(CFLAGS) -c -o $@ $< 


.PHONY: directories
directories:
	mkdir -p ./build ./bin

clean:
	rm -rf build

spotless: clean
	rm -rf bin
