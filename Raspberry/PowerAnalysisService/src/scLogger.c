/*
*  SlugCam's Energy Consumption Monitoring Service
*  Author: Kevin Abas
*  Date: 3/14/2015
*  Description: This file has normal C standard utility functions that check for error
*				return values and act on them accordingly.
*
*  INSERT LICENCE HERE
*/

#include	<errno.h>
#include	<syslog.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	"scLogger.h"


/* Error and log file handling */
static void multiarg_log(int errnoflag, int level, const char *fmt, va_list ap)
{
	int		errno_save, n;
	char	buf[MAXLINE + 1];

	errno_save = errno;		
#ifdef	HAVE_VSNPRINTF // check if supported
	vsnprintf(buf, MAXLINE, fmt, ap);	
#else
	vsprintf(buf, fmt, ap);				
#endif
	n = strlen(buf);
	if (errnoflag)
		snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));
	strcat(buf, "\n");
	syslog(level,"%s", buf);
	return;
}

/* Different logging functions for each level, ordered by severity */
void emerg_log(const char *fmt, ...) {
	va_list		ap;

	va_start(ap, fmt);
	multiarg_log(1, LOG_EMERG, fmt, ap);
	va_end(ap);
	exit(1);
}
void alert_log(const char *fmt, ...) {
	va_list		ap;

	va_start(ap, fmt);
	multiarg_log(1, LOG_ALERT, fmt, ap);
	va_end(ap);
	exit(1);
}
void crit_log(const char *fmt, ...) {
	va_list		ap;

	va_start(ap, fmt);
	multiarg_log(1,  LOG_CRIT, fmt, ap);
	va_end(ap);
	exit(1);
}
void err_log(const char *fmt, ...) {
	va_list		ap;

	va_start(ap, fmt);
	multiarg_log(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}
void warn_log(const char *fmt, ...) {
	va_list		ap;

	va_start(ap, fmt);
	multiarg_log(0, LOG_WARNING, fmt, ap);
	va_end(ap);
	exit(1);
}
void info_log(const char *fmt, ...) {
	va_list		ap;

	va_start(ap, fmt);
	multiarg_log(0, LOG_INFO, fmt, ap);
	va_end(ap);
	exit(1);
}
void debug_log(const char *fmt, ...) {
	va_list		ap;

	va_start(ap, fmt);
	multiarg_log(0, LOG_DEBUG, fmt, ap);
	va_end(ap);
	return;
}