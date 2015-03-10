#Makefile for PowerAnalysis-daemon

daemon: cJSON.h
	gcc -o PowerAnalysis-daemon cJSON.c PowerAnalysis-daemon.c #-lwiringPi

testClient:
	gcc -o PowerAnalysis-daemon-testClient PowerAnalysis-daemon-testClient.c

deploy:
	mv PowerAnalysis-daemon /usr/lib/SC-daemons/

clean:
	rm PowerAnalysis-daemon