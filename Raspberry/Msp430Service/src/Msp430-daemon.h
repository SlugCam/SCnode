/*
*  SlugCam's Msp430 Communications Service
*  Author: Kevin Abas
*  Date: 6/5/2015
*
*  Standard MIT License:
*  Copyright (c) 2015 SlugCam Team
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
* associated documentation files (the "Software"), to deal in the Software without restriction, including 
* without limitation the rights to use, copy, modify, merge, publish, distribute, sub-license, and/or sell 
* copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the 
* following conditions: The above copyright notice and this permission notice shall be included in all copies
* or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
* LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN 
* NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include 	<stdint.h>
#include	<stdarg.h>
#include 	<unistd.h>
#include	<errno.h>
#include 	<signal.h>
#include	<syslog.h>
#include	<string.h>


#ifndef Msp430Daemon__h
#define Msp430Daemon__h

#define		UNIXSOCKET_PATH "/tmp/mspunix.str" /* The Power Analysis daemon UNIX socket stream */
#define		MAXLINE			4096	/* Max text line length */
#define 	LISTENQUEUE		1024	/* Number of queued requests, default may be too small */
#define		SA				struct sockaddr		/* For ease */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* The Power Analysis request struct */
typedef struct paRequest {
	char *type;			/* Type of request: (stauts,..) */
	time_t timercvd;				/* The time at which the request was recieved, for internal use */
	char *data;				/* What data is being requested */
} paRequest;

/* Daemon related functions*/
//static void pabort(const char *s);
//static void transfer(int fd);

/* Unix Domain Socket handling functions */
ssize_t	wrap_write(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t wrap_readline(int fd, void *ptr, size_t maxlen);
void read_request(int sockfd) ;

/* Request handling functions */
int build_response(paRequest *curr_request, char *vptr_response);
int parse_request(paRequest *curr_request, const void *vptr_request);

#endif
