TARGETS=server
ALL_OBJS=$(TARGETS) 
LIB=lib_gutil.c event.c
CC=gcc
CCOPTS=-Wall -Wextra -g

.PHONY: all clean

all: $(ALL_OBJS)

$(TARGETS):%:%.c
	$(CC) $(CCOPTS) -o $@ $<

clean:
	rm -f $(ALL_OBJS)