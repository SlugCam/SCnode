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

all: directories
	${CC} ${CFLAGS} -c src/wifi_test.c -o build/wifi_test.o
	${CC} ${CFLAGS} -c src/wificmd.c -o build/wificmd.o
	${CC} ${CFLAGS} build/* -lwiringPi -lpthread -o bin/wifi_test

.PHONY: directories
directories:
	mkdir -p ./build ./bin

clean:
	rm -rf build

spotless: clean
	rm -rf bin
