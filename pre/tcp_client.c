#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int get_connected_fd(char* host_or_ip, char* port);
void chat_with_server(int connected_fd);

int main(int argc, char* argv[])
{
    int client_fd;

    client_fd = get_connected_fd(NULL, "21042");
    chat_with_server(client_fd);
    close(client_fd);

    exit(EXIT_SUCCESS);
}

int get_connected_fd(char* host_or_ip, char* port)
{
    int sockfd;
    int status;
    struct addrinfo hints;
    struct addrinfo* res, * rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // tcp
    if (host_or_ip == NULL) {
        hints.ai_flags = AI_PASSIVE;
    }

    status = getaddrinfo(NULL, port, &hints, &res);
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

    return sockfd;
}

void chat_with_server(int connected_fd)
{
    char buffer[1024] = { 0 };
    char* msg = "I'm a tcp client, let's chat.";

    send(connected_fd, msg, strlen(msg), 0);
    printf("msg client sent: %s\n", msg);
    read(connected_fd, buffer, 1024);
    printf("msg from server: %s\n", buffer);
    return;
}

