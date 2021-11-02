TARGETS=tcp_client tcp_server \
	graceclient graceserver addressused \
	select_test poll_server
DEP_TARGETS=test nonblocking_client nonblocking_server epoll_server \
    fork_server
PTH_OBJ=thread_test thread_server
ALL_OBJS=$(TARGETS) $(DEP_TARGETS) $(PTH_OBJ)
LIB=lib_gutil.c
CC=gcc
CCOPTS=-Wall -Wextra -g

.PHONY: all clean

all: $(ALL_OBJS)

$(TARGETS):%:%.c
	$(CC) $(CCOPTS) -o $@ $<

$(DEP_TARGETS):%:%.c $(LIB)
	$(CC) $(CCOPTS) $< $(LIB) -o $@

$(PTH_OBJ):%:%.c
	$(CC) $(CCOPTS) -pthread $(LIB) $< -o $@ 

clean:
	rm -f $(ALL_OBJS)