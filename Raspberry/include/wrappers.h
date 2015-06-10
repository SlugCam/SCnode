/*
*  SlugCam's Energy Consumption Monitoring Service
*  Author: Kevin Abas
*  Date: 3/14/2015
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
