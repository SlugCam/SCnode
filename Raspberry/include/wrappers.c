/*
*  SlugCam's Energy Consumption Monitoring Service
*  Author: Kevin Abas
*  Date: 3/14/2015
*  Description: This file has normal C standard utility functions that check for error
*				return values and act on them accordingly.
*
*  INSERT LICENCE HERE
*/

#include	<sys/socket.h>
#include	<stdio.h>
#include	<stdlib.h>
#include 	<unistd.h>
#include	<string.h>

#include	"scLogger.h"
#include	"wrappers.h"	


/*Generic Functions*/
pid_t wrap_fork(void) {
	pid_t	pid;

	if ( (pid = fork()) == -1)
		err_log("fork error");
	return(pid);
}
void wrap_close(int fd) {
	if (close(fd) == -1)
		err_log("close error");
}
char *wrap_strncpy( char *dest, const char *src, size_t count){
	int len;
	strncpy(dest, src, count);
	len = strlen(dest);
	printf("dest length:%d",len);
	if (dest[len] != '\0'){
		dest[len] = '\0';
	}
	return dest;
}

/* Socket Functions */
void wrap_bind(int fd, const struct sockaddr *sa, socklen_t salen) {
	if (bind(fd, sa, salen) < 0)
		err_log("bind error");
}
void wrap_connect(int fd, const struct sockaddr *sa, socklen_t salen) {
	if (connect(fd, sa, salen) < 0)
		err_log("connect error");
}
int wrap_socket(int fam, int type, int proto) {
	int	n;
	if ( (n = socket(fam, type, proto)) < 0)
		err_log("socket error");
	return(n);
}
void wrap_listen(int fd, int backlog) {
	char	*ptr;
	if ( (ptr = getenv("LISTENQUEUE")) != NULL)
		backlog = atoi(ptr);
	if (listen(fd, backlog) < 0)
		err_log("listen error");
}