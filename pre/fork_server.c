#include "lib_gutil.h"
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE 4096

void child_run(int fd)
{
    char outbuf[MAX_LINE + 1];
    size_t outbuf_used = 0;
    ssize_t result;

    while (1) {
        char ch;
        result = recv(fd, &ch, 1, 0);
        if (result == 0) {
            break;
        }
        else if (result == -1) {
            perror("recv");
            break;
        }

        if (outbuf_used < sizeof(outbuf)) {
            outbuf[outbuf_used++] = lib_rot13_char(ch);
        }

        if (ch == '\n') {
            send(fd, outbuf, outbuf_used, 0);
            outbuf_used = 0;
            continue;
        }
    }

    return;
}

void sigchld_handler(int sig)
{
    while (waitpid(-1, 0, WNOHANG) > 0);
    return;
}

int main(void)
{
    int listen_fd = tcp_listen_fd(NULL, "12345");
    struct sockaddr_storage cli_addr;
    socklen_t addr_len = sizeof(cli_addr);
    int new_fd;

    signal(SIGCHLD, sigchld_handler);
    printf("server: waitting for connections...\n");
    while (1) {
        new_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &addr_len);
        if (new_fd < 0) {
            report_error("accept failed.");
        }
        printf("server: got connection...\n");
        if (fork() == 0) {
            close(listen_fd);
            child_run(new_fd);
            // if(send(new_fd,"Hello,world!",13,0) == -1){
            //     perror("send");
            // }
            close(new_fd);
            exit(0);
        }
        close(new_fd); 
    }
    
    return 0;
}