#ifndef __LIB_GUTIL_H__
#define __LIB_GUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

int tcp_client(char* address, char* port);
int tcp_nonblocking_server(char* ser_port);

void report_error(char* err_msg);

#endif