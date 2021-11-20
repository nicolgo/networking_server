# networking_server
a project of networking protocol

## File structure
- `src`: all the code of the Http server, which include a Makefile and source code.
- `pre`: The study code, some test code of socket interfaces like select,poll, epoll and so on.
- `bgnet_examples` - the code of [`Beej's Guide to Network Programming`](https://beej.us/guide/bgnet/html/)
- `tutorial` - some test code.

## Test steps
1. change the location to `./src` dirtory.
2. using epoll dispatcher, make sure the event_loop->event_dispatcher in the `event_loop.c` equal to `&epoll_dispatcher`; using poll dispatcher, change the value to `&poll_dispatcher`.
3. using `make` to compile the code, then you will get an executable file
4. run the executable file, it will run the sercer in `localhost:21042`
5. open a new terminal window, using ab tool with `ab -n 10000 -c 100 -k http://127.0.0.1:21042/` command to test.

## Some Notes
1. using gdb debug the child process: `-exec set follow-fork-mode child`
2. check which process occupy one port: `lsof -i:<port>`, for instance, `lsof -i:8080`

## debug the core dump
1. check if system open dump core function by `ulimit -c`.
2. if the output is 0, using `ulimit -c unlimited`
3. install `coredumpctl` tool, the core dump file will in `/var/lib/systemd/coredump/`
4. examine the backtrace using gdb
 `#coredumpctl gdb app_name`  
 `(gdb) bt`  
  details can be seen in this [wiki](https://wiki.archlinux.org/title/Core_dump)