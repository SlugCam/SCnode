#Makefile for PowerAnalysis-daemon
INCLUDE = cJSON.h wrappers.h scLogger.h
LIBFILES = cJSON.c wrappers.c scLogger.c 
FLAGS = -Wall -Wstrict-prototypes -Wwrite-strings

daemon: $(INCLUDE) PowerAnalysis-daemon.h 
	gcc -o PowerAnalysis-daemon PowerAnalysis-daemon.c $(LIBFILES) $(FLAGS) #-lwiringPi

testClient: $(INCLUDE)
	gcc -o PowerAnalysis-daemon-testClient PowerAnalysis-daemon-testClient.c $(LIBFILES) $(FLAGS)

install:
	mv PowerAnalysis-daemon /usr/lib/SC-daemons/

clean:
	rm PowerAnalysis-daemon