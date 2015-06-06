#Makefile for PowerAnalysis-daemon
INCLUDE = ../../include
#LIBFILES = cJSON.c wrappers.c scLogger.c 
FLAGS = -Wall -Wstrict-prototypes -Wwrite-strings -I$(INCLUDE) -I/usr/local/include -L/usr/local/lib -lm -lwiringPi

daemon: $(INCLUDE) PowerAnalysis-daemon.h 
	gcc -o PowerAnalysis-daemon PowerAnalysis-daemon.c $(LIBFILES) $(FLAGS) 

testClient: $(INCLUDE)
	gcc -o PowerAnalysis-daemon-testClient PowerAnalysis-daemon-testClient.c $(LIBFILES) $(FLAGS)

install:
	mv PowerAnalysis-daemon /usr/lib/SC-daemons/

clean:
	rm -f PowerAnalysis-daemon *.o
