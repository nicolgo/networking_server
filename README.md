# networking_server
a project of networking protocol
## Test steps
`ab -n 10000 -c 100 http://127.0.0.1:21042/`

## Some Notes
1. using gdb debug the child process: `-exec set follow-fork-mode child`
2. check which process occupy one port: `lsof -i:<port>`, for instance, `lsof -i:8080`

## debug the core dump
1. check if system open dump core function by `ulimit -c`.
2. if the output is 0, using `ulimit -c unlimited`
3. install `coredumpctl` tool, the core dump file will in `/var/lib/systemd/coredump/`