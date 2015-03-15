/*
*  SlugCam's Energy Consumption Monitoring Service
*  Author: Kevin Abas
*  Date: 3/8/2015
*  Description: This is SlugCam's Power Analysis Daemon that not only monitors battery 
*				life and status, but also processes requests from other daemons and responds
*				with status messages.
*
*  INSERT LICENCE HERE
*/



#include	<sys/un.h>
#include	<sys/socket.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include 	<unistd.h>
#include	<errno.h>
#include 	<signal.h>
#include	<syslog.h>
#include	<strings.h>
#include	<time.h>

#include 	"cJSON.h" 
#include	"wrappers.h"
#include	"scLogger.h"
#include	"PowerAnalysis-daemon.h"
//#include 	<wiringPi.h>


#ifndef	AF_LOCAL
#define AF_LOCAL	AF_UNIX // system may not support AF_LOCAL yet
#endif		



/* Socket I/O functions */
ssize_t	wrap_write(int fd, const void *vptr, size_t n) {
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		
			else
				return(-1);			
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	if (nwritten != n)
		err_log("write error");
	return(n);
}
int parseRequest( const void *vptr, size_t n){
	const char	*ptr;
	cJSON 		*root;
	paRequest 	*curr_request;

	ptr = vptr;
	if ( (root = cJSON_Parse(ptr)) == 0){
		return -1;
	}else{
		curr_request->type= cJSON_GetObjectItem(root,"type")->valuestring;
		curr_request->data= cJSON_GetObjectItem(root,"data")->valuestring;
		curr_request->timercvd= time(&curr_request->timercvd);
	}
	return 1;

}
static ssize_t readBytes(int fd, char *ptr) {
	static int	read_cnt;
	static char	*read_ptr;
	static char	read_buf[MAXLINE];

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR) /* EINTR = interupt syscall */
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}
ssize_t readline(int fd, void *vptr, size_t maxlen) {
	ssize_t	n, rc;
	char	c, *ptr;
	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = readBytes(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);
		} else
			return(-1);	
	}

	*ptr = 0;	
	return(n);
}
ssize_t Readline(int fd, void *ptr, size_t maxlen) {
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_log("readline error");
	return(n);
}
void read_request(int sockfd) {
    ssize_t     n;
    char        request[MAXLINE];
 
    while(1) {
        if ( (n = Readline(sockfd, request, MAXLINE)) == 0)
            return;  /* No rquest was recieved */
 		
        if (parseRequest(request, MAXLINE) < 0){
			warn_log("Nothing Parsed, Incorrect JSON format");
			return;
		}

		//TODO: MAKE JSON RESPONSE AND SEND IT BACK
        //wrap_write(sockfd, request, n);
    }
}


/* Structure follows format found in the text:
   "Unix Network Programming" by W. Richard Stevens */
int main(int argc, char **argv) {
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_un	cliaddr, servaddr;

	listenfd = wrap_socket(AF_LOCAL, SOCK_STREAM, 0);

	unlink(UNIXSOCKET_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSOCKET_PATH);

	wrap_bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	wrap_listen(listenfd, LISTENQUEUE);

	while(1) {
		clilen = sizeof(cliaddr);
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;
			else
				err_log("accept error");
		}

		if ( (childpid = wrap_fork()) == 0) {	
			wrap_close(listenfd);	/* Close listening socket on cloned process */
			read_request(connfd);	/* process request */
			exit(0);
		}
		wrap_close(connfd);			/* parent closes connected socket */
	}
}