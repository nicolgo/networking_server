# networking_server
a project of networking protocol


# Some Note
1. using gdb debug the child process: `-exec set follow-fork-mode child`
2. check which process occupy one port: `lsof -i:<port>`, for instance, `lsof -i:8080`