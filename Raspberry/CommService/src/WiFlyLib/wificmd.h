int initializeModule();

int disableHelloMessage();

int serialReceive(char * response, int serialLine );

int checkCmdSyntax(char * response);

int cmdModeEnable(char * response, int serialLine);

int cmdModeDisable(char * response, int serialLine);

int connectWifi();

int openConnection(int fd, char* address, char* port);

int closeConnection(int fd);

