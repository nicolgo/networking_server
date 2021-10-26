#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#define MAXLINE 1024

int get_sockfd(char* address, char* port);

int main(int argc, char* argv[])
{
    int sockfd;
    char recv_line[MAXLINE], send_line[MAXLINE];
    int n;

    sockfd = get_sockfd(NULL, "43211");

    fd_set readmask;
    fd_set allreads;
    FD_ZERO(&allreads);
    FD_SET(0, &allreads);//set 0 fd to 1
    FD_SET(sockfd, &allreads);// set sockfd to 1

    while (1) {
        readmask = allreads;
        int rc = select(sockfd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0) {
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(sockfd, &readmask)) {
            n = read(sockfd, recv_line, MAXLINE);
            if (n < 0 || n == 0) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        if(FD_ISSET(STDIN_FILENO,&readmask)){
            if(fgets(send_line,MAXLINE,stdin) != NULL){
                int i = strlen(send_line);
                if(send_line[i-1] == '\n'){
                    send_line[i-1] = 0;
                }
                printf("now sending %s\n",send_line);
                ssize_t rt = write(sockfd,send_line,strlen(send_line));
                if(rt < 0){
                    perror("write failed!\n");
                    exit(EXIT_FAILURE);
                }
                printf("send bytes: %zu \n",rt);
            }
        }

    }
    exit(EXIT_SUCCESS);
}

int get_sockfd(char* address, char* port)
{
    int sockfd;
    int status;
    struct addrinfo hints, * res, * rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (address == NULL) {
        hints.ai_flags = AI_PASSIVE;
    }

    status = getaddrinfo(address, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            perror("create socket failed!\n");
            continue;
        }
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == -1) {
            perror("connect failed!\n");
            close(sockfd);
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        perror("failed to connect");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}