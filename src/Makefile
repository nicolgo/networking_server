TARGET=my_app

SRCS = $(wildcard *.c)

CC=gcc
CFLAGS=-Wall -Wextra -g

.PHONY: all clean

all: $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)