/*
*  SlugCam's Energy Consumption Monitoring Service
*  Author: Kevin Abas
*  Date: 3/15/2015
*
*  INSERT LICENCE HERE
*/


#ifndef PowerAnalysisDaemon__h
#define PowerAnalysisDaemon__h

#define		UNIXSOCKET_PATH "/tmp/paunix.str" /* The Power Analysis daemon UNIX socket stream */
#define		MAXLINE		4096	/* max text line length */
#define 	LISTENQUEUE	1024	/* Number of queued requests, default may be too small */
#define	SA	struct sockaddr		/* For ease */

/* The Power Analysis request struct */
typedef struct paRequest {
	char *type;			/* Type of request: (stauts,..) */
	time_t timercvd;				/* The time at which the request was recieved, for internal use */
	char *data;				/* What data is being requested */
} paRequest;

/* Unix Domain Socket handling functions */
ssize_t	wrap_write(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t Readline(int fd, void *ptr, size_t maxlen) ;
void read_request(int sockfd) ;

/* Request handling functions */
int parseRequest( const void *vptr, size_t n);


#endif