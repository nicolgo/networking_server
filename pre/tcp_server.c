#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define PORT "12345"
#define BACKLOG 10

int main(int argc, char* argv[])
{
    int status;
    int sfd;
    int new_sfd;
    struct addrinfo hints;
    struct addrinfo* res, * rp;
    char buffer[1024] = { 0 };
    char* msg = "message from server";
    struct sockaddr_storage their_addr;
    int sin_size = sizeof their_addr;
        
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close(sfd);
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    if (listen(sfd, BACKLOG) == -1) {
        fprintf(stderr, "Could not listen\n");
        exit(EXIT_FAILURE);
    }

    if ((new_sfd = accept(sfd, (struct sockaddr *)&their_addr,&sin_size)) < 0) {
        fprintf(stderr, "Could not accept");
        exit(EXIT_FAILURE);
    }
    read(new_sfd, buffer, 1024);
    printf("server read client message: %s\n", buffer);
    send(new_sfd, msg, strlen(msg), 0);
    printf("server sent message: %s\n",msg);

    exit(EXIT_SUCCESS);
}