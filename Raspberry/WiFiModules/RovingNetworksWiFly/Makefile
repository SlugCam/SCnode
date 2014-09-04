# Author: Kevin Abas

CC="../../../toolchain/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc"
CXX="../../../toolchain/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-g++"
LD="../../../toolchain/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-ld"
BUILDROOT_STAGING_DIR=../../../toolchain/staging
BUILDROOT_TARGET_DIR=../../../toolchain/target
CFLAGS= -Wall -I "$(BUILDROOT_STAGING_DIR)/usr/include" \
		-I "$(BUILDROOT_STAGING_DIR)/usr/include/wiringPi" \
		-I "$(BUILDROOT_STAGING_DIR)/opt/vc/include" \
	    -L "$(BUILDROOT_STAGING_DIR)/opt/vc/lib" \
	    -L "$(BUILDROOT_STAGING_DIR)/usr/lib" \
	    -L "$(BUILDROOT_STAGING_DIR)/lib" \
	    -L "$(BUILDROOT_TARGET_DIR)/opt/vc/lib" \
	    -L "$(BUILDROOT_TARGET_DIR)/usr/lib" \
	    -L "$(BUILDROOT_TARGET_DIR)/lib" 
DEPS = wificmd.h
OBJ = wifiModule.o wificmd.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

wificmd: wificomm.o 
	$(CC) -o wificomm wificomm.o comm_func.o -I.

debugUART: debugUART.o
	$(CC) -o debugUART debugUART.o -lwiringPi -lpthread

wifiModule: $(OBJ)
	$(CC) -o $@ $^  -lwiringPi -lpthread

cJSONtest: cJSONtest.o
	$(CC) -o cJSONtest cJSONtest.o -lcJSON

clean:
	rm -f wificomm wificomm.o comm_func.o