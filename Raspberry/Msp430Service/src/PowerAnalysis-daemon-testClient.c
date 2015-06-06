/*
*  The PowerAnalysis testClient
*  Author: Kevin Abas
*  Date: 3/8/2015
*  Description: Purpose is to test request and responses from
*               PowerAnalysis Daemon.
*/



#include	<sys/un.h>
#include	<sys/socket.h>
#include	<stdio.h>
#include	<stdlib.h>
#include 	<unistd.h>
#include	<errno.h>
#include	<strings.h>

#include	"wrappers.h"
#include 	"cJSON.h"
#include	"scLogger.h"


#define		UNIXSOCKET_PATH "/tmp/paunix.str"
#define		MAXLINE			4096	/* max text line length */
#define		SA				struct sockaddr

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
/* Function that reads from a socket byte stream follows format from text:
   "Unix Network Programing" by W. Richard Stevens */
static ssize_t read_bytes(int fd, char *ptr) {
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

/* function that fills request string buffer with bytes from socket stream */
ssize_t readline(int fd, void *vptr, size_t maxlen) {
	ssize_t	n, rc;
	char	c, *ptr;
	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = read_bytes(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\0')
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

/* readline function wrapper to catch errors  */
ssize_t wrap_readline(int fd, void *ptr, size_t maxlen) {
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_log("readline error");
	return(n);
}

void sendJSON(FILE *fp, int sockfd) {
	char	sendline[MAXLINE], recvline[MAXLINE];
	char	testJSON[256] = "{\n\"type\": \"status-request\",\n \"data\": \"battery\"\n }\r";

	strcpy(sendline, testJSON);

	wrap_write(sockfd, sendline, strlen(sendline));

	if (wrap_readline(sockfd, recvline, MAXLINE) == 0)
		printf("sendJSON: server terminated prematurely");

	fputs(recvline, stdout);	
}

int main(int argc, char **argv) {
	int					sockfd;
	struct sockaddr_un	servaddr;

	sockfd = wrap_socket(AF_LOCAL, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSOCKET_PATH);

	wrap_connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	sendJSON(stdin, sockfd);	

	exit(0);
}
