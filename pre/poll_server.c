#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

#define CLIENT_SIZE 128
#define MAXLINE 1024

int get_listenfd();

int main(int argc, char* argv[])
{
    int ready_number;
    struct sockaddr_storage cli_addr;
    socklen_t cli_addlen;
    int connected_fd;
    char buf[MAXLINE];
    int listenfd = get_listenfd();

    struct pollfd event_set[CLIENT_SIZE];
    event_set[0].fd = listenfd;
    event_set[0].events = POLLRDNORM;
    // ignore unused pollfd
    int i;
    for (i = 1;i < CLIENT_SIZE;i++) {
        event_set[i].fd = -1;
    }

    for (;;) {
        // set CLIENT_SIZE since poll can gurantee the fd==-1 will be ignored
        // timeout==-1 means poll will block.
        if ((ready_number = poll(event_set, CLIENT_SIZE, -1)) < 0) {
            perror("failed to poll");
            exit(EXIT_FAILURE);
        }

        if (event_set[0].revents & POLLRDNORM) {
            connected_fd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_addlen);
            // register a new socket
            for (i = 1;i < CLIENT_SIZE;i++) {
                if (event_set[i].fd < 0) {
                    event_set[i].fd = connected_fd;
                    event_set[i].events = POLLRDNORM;
                    break;
                }
            }
            if (i == CLIENT_SIZE) {
                perror("cannot hold so many clients");
                exit(EXIT_FAILURE);
            }
            if (--ready_number <= 0) {
                continue;
            }
        }

        for (i = 1;i < CLIENT_SIZE;i++) {
            int socket_fd;
            ssize_t n;
            if ((socket_fd = event_set[i].fd) < 0) {
                continue;
            }
            if (event_set[i].revents & (POLLRDNORM | POLLERR)) {
                if ((n = read(socket_fd, buf, MAXLINE)) > 0) {
                    if (write(socket_fd, buf, n) < 0) {
                        perror("write error");
                    }
                }
                else if (n == 0 || errno == ECONNRESET) {
                    close(socket_fd);
                    event_set[i].fd = -1;
                }
                else {
                    perror("read error");
                    exit(EXIT_FAILURE);
                }

                if (--ready_number <= 0) {
                    break;
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}

int get_listenfd()
{
    int listenfd;
    int status;
    struct addrinfo hints, * res, * rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, "12345", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "%s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            perror("failed to create socket");
            exit(EXIT_FAILURE);
        }
        if ((bind(listenfd, rp->ai_addr, rp->ai_addrlen)) == 0) {
            break;
        }
        close(listenfd);
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        perror("failed to bind");
        exit(EXIT_FAILURE);
    }
    if (listen(listenfd, 10) == -1) {
        perror("failed to listen");
        exit(EXIT_FAILURE);
    }

    return listenfd;
}