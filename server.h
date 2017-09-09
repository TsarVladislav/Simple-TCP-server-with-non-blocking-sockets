#ifndef _NON_BLOCK_SERVER_H_
#define _NON_BLOCK_SERVER_H_
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#define BACKLOG 10
#define BUFSIZE 256

void intHandler(int dummy);
int create_connection(char *port);
int resize(struct pollfd **fds, int size);
int newcon(struct pollfd *fds, int i, int fds_size);
void polen(struct pollfd fds, char *fp, int i);
void servmsg(struct pollfd fds, char *fp, int i);
char *strret(char *str);
#endif
