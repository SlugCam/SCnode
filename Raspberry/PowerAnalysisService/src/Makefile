#Makefile for PowerAnalysis-daemon

daemon: cJSON.h wrappers.h scLogger.h
	gcc -o PowerAnalysis-daemon cJSON.c PowerAnalysis-daemon.c wrappers.c scLogger.c #-lwiringPi

testClient:
	gcc -o PowerAnalysis-daemon-testClient PowerAnalysis-daemon-testClient.c

install:
	mv PowerAnalysis-daemon /usr/lib/SC-daemons/

clean:
	rm PowerAnalysis-daemon