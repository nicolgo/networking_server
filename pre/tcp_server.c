#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int get_server_listen_fd(char* host_or_ip, char* port);
void chat_with_client(int accpet_fd);


int main(int argc, char* argv[])
{
    int new_sfd;
    int listen_fd;
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof client_addr;

    listen_fd = get_server_listen_fd(NULL, "21042");
    if ((new_sfd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
        perror("failed to accept");
        exit(EXIT_FAILURE);
    }
    chat_with_client(new_sfd);
    close(new_sfd);
    close(listen_fd);
    exit(EXIT_SUCCESS);
}

int get_server_listen_fd(char* host_or_ip, char* port)
{
    int status;
    int listen_fd;
    struct addrinfo hints;
    struct addrinfo* res, * rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (host_or_ip == NULL) {
        hints.ai_flags = AI_PASSIVE;// if no ip, fill in localhost.
    }
    if ((status = getaddrinfo(host_or_ip, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close(listen_fd);
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        perror("failed to bind\n");
        exit(EXIT_FAILURE);
    }
    if (listen(listen_fd, 10) == -1) {
        perror("failed to listen\n");
        exit(EXIT_FAILURE);
    }

    return listen_fd;
}

void chat_with_client(int accpet_fd)
{
    char buffer[1024] = { 0 };
    char* msg = "I'm a tcp server, let's chat.";

    read(accpet_fd, buffer, 1024);
    printf("msg client sent: %s\n", buffer);
    send(accpet_fd, msg, strlen(msg), 0);
    printf("msg server sent: %s\n", msg);

    return;
}
