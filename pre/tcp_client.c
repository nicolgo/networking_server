#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define PORT "12345"

void send_data(int sockfd)
{
    return;
}

int main(int argc, char* argv[])
{
    int sockfd;
    int status;
    struct addrinfo hints;
    struct addrinfo* res, * rp;
    char buffer[1024] = { 0 };
    char* msg = "message from client";

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, PORT, &hints, &res);
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
    send(sockfd, msg, strlen(msg), 0);
    printf("client sent message: %s\n",msg);
    read(sockfd, buffer, 1024);
    printf("read message from server: %s\n", buffer);
    exit(EXIT_SUCCESS);
}