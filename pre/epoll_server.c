#include "lib_gutil.h"
#include <sys/epoll.h>

#define MAX_EVENTS 128

int main(int argc, char* argv[])
{
    int listen_fd, tmp_sock;
    int efd;
    int n, i;
    struct epoll_event event;
    struct epoll_event* events;

    listen_fd = tcp_nonblocking_server(NULL, "12345");

    efd = epoll_create1(0);
    if (efd == -1) {
        report_error("epoll create failed");
    }

    event.data.fd = listen_fd;
    event.events = EPOLLIN | EPOLLET;//EPOLLIN-readable;EPOLLET-edge_triggerrd
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
        report_error("epoll failed to add listen fd");
    }

    events = calloc(MAX_EVENTS, sizeof(event));

    while (1) {
        n = epoll_wait(efd, events, MAX_EVENTS, -1);
        printf("epoll_wait wakeup\n");
        for (i = 0;i < n;i++) {
            if ((events[i].events & EPOLLERR) || events[i].events & EPOLLHUP
                || (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if (listen_fd == events[i].data.fd) {
                struct sockaddr_storage cli_addr;
                socklen_t addr_len = sizeof(cli_addr);
                int fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &addr_len);
                if (fd < 0) {
                    report_error("accept failed");
                }
                else {
                    fcntl(fd, F_SETFL, O_NONBLOCK);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET;//edge-triggered
                    /*level-triggered. if data in buffer, keep delivering event.*/
                    //event.events = EPOLLIN;
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
                        report_error("epoll_ctl failed");
                    }
                }
                continue;
            }
            else {
                tmp_sock = events[i].data.fd;
                printf("get event on socket fd == %d \n", tmp_sock);
                while (1) {
                    char buf[512];
                    if ((n = read(tmp_sock, buf, sizeof(buf))) < 0) {
                        if (errno != EAGAIN) {
                            close(tmp_sock);
                            report_error("read error");
                        }
                        break;
                    }
                    else if (n == 0) {
                        close(tmp_sock);
                        break;
                    }
                    else {
                        for (i = 0;i < n;i++) {
                            buf[i] = lib_rot13_char(buf[i]);
                        }
                        if (write(tmp_sock, buf, n) < 0) {
                            report_error("write error");
                        }
                    }
                }
            }
        }
    }

    free(events);
    close(listen_fd);
    exit(EXIT_SUCCESS);
}