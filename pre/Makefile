TARGETS=tcp_client tcp_server \
	graceclient graceserver addressused \
	select_test

CC=gcc
CCOPTS=-Wall -Wextra

.PHONY: all clean pristine

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

pristine: clean

%: %.c
	$(CC) $(CCOPTS) -o $@ $<
