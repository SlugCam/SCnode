/*
*  SlugCam's Msp430 Communications Service
*  Author: Kevin Abas
*  Date: 6/5/2015
*
*  INSERT LICENCE HERE
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
static void pabort(const char *s);
static void transfer(int fd)

/* Unix Domain Socket handling functions */
ssize_t	wrap_write(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t wrap_readline(int fd, void *ptr, size_t maxlen);
void read_request(int sockfd) ;

/* Request handling functions */
int build_response(paRequest *curr_request, char *vptr_response);
int parse_request(paRequest *curr_request, const void *vptr_request);

#endif
