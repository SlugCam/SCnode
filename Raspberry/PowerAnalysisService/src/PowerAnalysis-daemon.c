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
#include	<string.h>
#include	<time.h>

#include 	"cJSON.h" 
#include	"wrappers.h"
#include	"scLogger.h"
#include	"PowerAnalysis-daemon.h"
#include 	<wiringPi.h>


#ifndef	AF_LOCAL
#define AF_LOCAL	AF_UNIX // system may not support AF_LOCAL yet
#endif		


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

const char  *getBatteryStatus(){
	if (wiringPiSetupSys() == -1){
    	warn_log("Couldn't setup WiringPi Library");
    	return "ERROR with WiringPi";
    }

    pinMode (DONEPIN,INPUT);
    pullUpDnControl (DONEPIN, PUD_UP);
    pinMode (CHRGPIN,INPUT);
    pullUpDnControl (CHRGPIN, PUD_UP);
    pinMode (PGOODPIN,INPUT);
    pullUpDnControl (CHRGPIN, PUD_UP);

    /*
    * Determine battery status From Adafruit's LiPo Charger:
    *	D 	C 	PG 		Satus:
    *   1   	0   	0 		Battery Charging Complete
    *   1	1	0 		No Battery Present
    *   1 	1 	1 		No Input Power Present
    * 	0 	0 	0 		Temperature/Timer Fault
    *   0	1 	0 		Charging (constant Voltage/Current)
    * 	0 	1 	1    	Low Battery 
    *
    */
    if (digitalRead(DONEPIN) == 1){
    	if (digitalRead(CHRGPIN) == 0){
    		return "Battery Charging Complete.";
    	} else {
    		if (digitalRead(PGOODPIN) == 0){
    			return "No Battery Present.";
    		} else {
    			return "No Input Power Present.";
    		}
    	}
    } else {
    	if (digitalRead(CHRGPIN) == 0){
    		return "Temperature/Timer Fault.";
    	} else {
    		if (digitalRead(CHRGPIN) == 0){
    			return "Charging.";
    		} else {
    			return "Low Battery.";
    		}

    	}
    }


    return "ERROR READING STATUS";	
}

/* Builds JSON response according to paRequest contents */
int build_response(paRequest *curr_request, char *ptr_response){
	cJSON 			*root, *data;
	time_t 			timegen;
	struct tm 		*loctime;
	char 			*timestr,*jsonPrint;

	root = cJSON_CreateObject();  
	cJSON_AddStringToObject(root, "type", "message");
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());

	if (strcmp(curr_request->type, "status-request") == 0){
		if (strcmp(curr_request->data, "battery") == 0){
			cJSON_AddStringToObject(data, "type", "battery");
			cJSON_AddStringToObject(data, "battery", getBatteryStatus());
		} else if (strcmp(curr_request->data, "consumption") == 0) {
			cJSON_AddStringToObject(data, "type", "consumption");
			//call function here to read current sensor, but for now fake value
			cJSON_AddStringToObject(data, "avgCurrent", "467");
		} else if (strcmp(curr_request->data, "remain") == 0) {
			cJSON_AddStringToObject(data, "type", "remainingTime");
			//call function here to read remaining time left on current battery charge
			cJSON_AddStringToObject(data, "remainingTime", "124009");
		} else {
			cJSON_AddStringToObject(data, "type", "error");
			cJSON_AddStringToObject(data, "message", "Requested status type not recognized.");
			warn_log("Recieved unkown status request type.");
		}

		timegen = time(NULL);
		loctime = localtime (&timegen);
		timestr = asctime (loctime);
		cJSON_AddStringToObject(data, "timeGenerated", timestr);
		jsonPrint = cJSON_Print(root);
		strcpy(ptr_response,jsonPrint);
		free(jsonPrint);
		cJSON_Delete(root);
		return 1;
		
	}else{
		cJSON_AddStringToObject(data, "type", "error");
		cJSON_AddStringToObject(data, "message", "Request type not recognized.");
		warn_log("Recieved unkown request type.");
		
		timegen = time(NULL);
		loctime = localtime (&timegen);
		timestr = asctime (loctime);
		cJSON_AddStringToObject(data, "timeGenerated", timestr);
		jsonPrint = cJSON_Print(root);
		strcpy(ptr_response,jsonPrint);
		free(jsonPrint);
		cJSON_Delete(root);
		return 1;
	}
}

/* Parses JSON request from request string and creates paRequest struct */
int parse_request(paRequest *curr_request, const void *vptr_request){
	const char		*ptr;
	cJSON 			*root;
	char			*jsonPrint;
	
	ptr = vptr_request;
	root = cJSON_Parse(ptr);

	if (!root){
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}else{
		jsonPrint = cJSON_Print(root);
		debug_log("Request recieved:%s\n",jsonPrint);
		free(jsonPrint);
		curr_request->type = malloc(sizeof(char)*30);
		strcpy(curr_request->type, cJSON_GetObjectItem(root,"type")->valuestring);
		curr_request->data = malloc(sizeof(char)*30);
		strcpy(curr_request->data, cJSON_GetObjectItem(root,"data")->valuestring);
		curr_request->timercvd = time(NULL);
	}
	cJSON_Delete(root);
	return 1;

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
			if (c == '\r')
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

/* function that handles request from socket stream*/
void read_request(int sockfd) {
    ssize_t     n;
    char        request[MAXLINE];
    char		response[MAXLINE];
    paRequest 	par;
 	
    if ( (n = wrap_readline(sockfd, request, MAXLINE)) == 0){
    	debug_log("Finished reading bytes from stream.");
        return; 
    }

    if (parse_request(&par, request) < 0){
		warn_log("Nothing Parsed, incorrect JSON format.");
		return;
	}
	if (build_response(&par, response) < 0){
		warn_log("Couldn't build JSON response.");
		return;
	}

	//Free allocated memory for request struct
	free(par.type);
	free(par.data);
	
	n = strlen(response);
	if (wrap_write(sockfd, response, n) < 0){
		debug_log("Unable to send response.");
	}

}


/* Structure follows format found in the text:
   "Unix Network Programing" by W. Richard Stevens */
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
