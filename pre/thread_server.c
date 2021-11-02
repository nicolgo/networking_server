#include "lib_gutil.h"
#include <pthread.h>

#define MAX_LINE 16384

void loop_echo(int);

void *thread_run(void *arg)
{
    pthread_detach(pthread_self());
    int fd = (int)arg;
    loop_echo(fd);
}

int main(void)
{
    pthread_t thid;
    int new_fd;
    int listen_fd = tcp_listen_fd(NULL,"12345");
    
    while(1){
        new_fd = get_accept_fd(listen_fd);
        if(new_fd < 0){
            report_error("accept failed");
        }else{
            pthread_create(&thid,NULL,thread_run,(void*)new_fd);
        }
    }
    exit(EXIT_SUCCESS);
}


void loop_echo(int fd)
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
            perror("read error");
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
}