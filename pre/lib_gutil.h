#ifndef __LIB_GUTIL_H__
#define __LIB_GUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int tcp_client(char* address, char* port);

#endif