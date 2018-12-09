#ifndef HTTPSOCKET_H
#define HTTPSOCKET_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <termios.h>    /*PPSIX ÷’∂Àøÿ÷∆∂®“Â*/
#include <iconv.h>

#include <string>
#include <map>
#include <list>
#include <vector>
#include <ostream>
#include <fstream>
#include <ios>
#include <sstream>

int Connect(int fd, struct sockaddr *addr, socklen_t len);
int Socket_Connect(char *ip,char *port);
int Send(int sockfd, char *sendbuf, int len, int flags);
int Close(int sockfd);
int Recv(int sockfd, char *recvbuf, int len, int flags);

#endif
