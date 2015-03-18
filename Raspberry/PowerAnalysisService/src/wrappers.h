/*
*  SlugCam's Energy Consumption Monitoring Service
*  Author: Kevin Abas
*  Date: 3/14/2015
*
*  INSERT LICENCE HERE
*/


#ifndef wrappers__h
#define wrappers__h

/* Generic Functions */
pid_t wrap_fork(void);
void wrap_close(int fd);
void wrap_waitpid(void);
char *wrap_strncpy( char *dest, const char *src, size_t count);

/* Socket Related Functions */
void wrap_connect(int fd, const struct sockaddr *sa, socklen_t salen);
void wrap_bind(int fd, const struct sockaddr *sa, socklen_t salen);
int wrap_socket(int fam, int type, int proto);
void wrap_listen(int fd, int backlog);

#endif
