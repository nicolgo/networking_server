#ifndef LIB_GUTIL_H
#define LIB_GUTIL_H

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <poll.h>
#include <assert.h>
#include <pthread.h>
#include "lib_log.h"
#include "channel.h"
#include "event_dispatcher.h"
#include "event_loop.h"
#include "buffer.h"
#include "thread_pool.h"
#include "lib_tcp_server.h"
#include "lib_http.h"


#define SERVER_PORT 21042

#endif