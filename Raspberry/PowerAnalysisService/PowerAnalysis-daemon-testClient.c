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

#define		UNIXSOCKET_PATH "/tmp/paunix.str"
#define		MAXLINE			4096	/* max text line length */
#define		SA				struct sockaddr

/* Global variables for now, will move them*/
static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];

/* Wrappers for error handling */
void uConnect(int fd, const struct sockaddr *sa, socklen_t salen) {
	if (connect(fd, sa, salen) < 0)
		printf("connect error");
}
int uSocket(int fam, int type, int proto) {
	int	n;
	if ( (n = socket(fam, type, proto)) < 0)
		printf("socket error");
	return(n);
}

/* Socket I/O functions */
ssize_t	uWrite(int fd, const void *vptr, size_t n) {
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
		printf("write error");
	return(n);
}
static ssize_t my_read(int fd, char *ptr) {

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
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
		if ( (rc = my_read(fd, &c)) == 1) {
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
ssize_t readlinebuf(void **vptrptr) {
	if (read_cnt)
		*vptrptr = read_ptr;
	return(read_cnt);
}
ssize_t Readline(int fd, void *ptr, size_t maxlen) {
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		printf("readline error");
	return(n);
}

void str_cli(FILE *fp, int sockfd) {
	char	sendline[MAXLINE], recvline[MAXLINE];

	while (fgets(sendline, MAXLINE, fp) != NULL) {

		uWrite(sockfd, sendline, strlen(sendline));

		if (Readline(sockfd, recvline, MAXLINE) == 0)
			printf("str_cli: server terminated prematurely");

		fputs(recvline, stdout);
	}
}

int main(int argc, char **argv) {
	int					sockfd;
	struct sockaddr_un	servaddr;

	sockfd = uSocket(AF_LOCAL, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSOCKET_PATH);

	uConnect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	str_cli(stdin, sockfd);		/* do it all */

	exit(0);
}