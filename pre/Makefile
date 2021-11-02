TARGETS=tcp_client tcp_server \
	graceclient graceserver addressused \
	select_test poll_server
DEP_TARGETS=test nonblocking_client nonblocking_server epoll_server
LIB=lib_gutil.c
CC=gcc
CCOPTS=-Wall -Wextra -g

.PHONY: all clean pristine

all: $(TARGETS) $(DEP_TARGETS)


pristine: clean

$(TARGETS):%:%.c
	$(CC) $(CCOPTS) -o $@ $<

$(DEP_TARGETS):%:%.c $(LIB)
	$(CC) $(CCOPTS) $< $(LIB) -o $@

clean:
	rm -f $(TARGETS)
	rm -f $(DEP_TARGETS)