/*
*  SlugCam's Energy Consumption Monitoring Service
*  Author: Kevin Abas
*  Date: 3/14/2015
*
*  INSERT LICENCE HERE
*/

#ifndef scLogger__h
#define scLogger__h


#define		MAXLINE		4096	/* max text line length */

static void multiarg_log(int errnoflag, int level, const char *fmt, va_list ap);

/* Different logging functions for each level, ordered by severity */
void emerg_log(const char *fmt, ...);
void alert_log(const char *fmt, ...);
void crit_log(const char *fmt, ...);
void err_log(const char *fmt, ...);
/* Log msgs that don't set off errno */
void warn_log(const char *fmt, ...);
void info_log(const char *fmt, ...);
void debug_log(const char *fmt, ...);

#endif