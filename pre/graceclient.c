#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define SERVPORT "43211"
#define MAX_LINE 4096

int main(int argc, char* argv[])
{
    int sockfd;
    int status;
    struct addrinfo hints, * res, * rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;//v4 or v6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, SERVPORT, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }
        close(sockfd);
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    // .......
    char send_line[MAX_LINE], recv_line[MAX_LINE];
    int n;
    fd_set readmask;
    fd_set allreads;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(sockfd, &allreads);

    while (1) {
        readmask = allreads;
        int rc = select(sockfd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0) {
            fprintf(stderr, "select failed\n");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(sockfd, &readmask)) {
            n = read(sockfd, recv_line, MAX_LINE);
            if (n < 0) {
                fprintf(stderr, "read error\n");
            }
            else if (n == 0) {
                fprintf(stderr, "server terminated \n");
            }
            recv_line[0] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        if (FD_ISSET(0, &readmask)) {
            if (fgets(send_line, MAX_LINE, stdin) != NULL) {
                if (strncmp(send_line, "shutdown", 8) == 0) {
                    FD_CLR(0, &allreads);
                    if (shutdown(sockfd, 1)) {
                        fprintf(stderr, "shutdown failed");
                    }
                }
                else if (strncmp(send_line, "close", 5) == 0) {
                    FD_CLR(0, &allreads);
                    if (close(sockfd)) {
                        fprintf(stderr, "close failed");
                    }
                    sleep(6);
                    exit(0);
                }
                else {
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n') {
                        send_line[i - 1] = 0;
                    }
                    printf("now sending %s\n", send_line);
                    size_t rt = write(sockfd, send_line, strlen(send_line));
                    if (rt < 0) {
                        fprintf(stderr, "write failed");
                    }
                    printf("send bytes: %zu \n", rt);
                }
            }
        }
    }

    exit(EXIT_SUCCESS);
}