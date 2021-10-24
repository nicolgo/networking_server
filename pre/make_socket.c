#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int make_socket(uint16_t port){
    int sock;
    struct sockaddr_in name;

    // creat IPV4 socker.
    sock = socket(PF_INET,SOCK_STREAM,0);
    if(sock < 0){
        perror("create socker failed.");
        exit(EXIT_FAILURE);
    }

    //bind port and ip
    name.sin_family = AF_INET;
    name.sin_port = htons(port); // set port
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock,(struct sockaddr *) &name, sizeof(name)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int main(int argc, char **argv){
    int sockfd = make_socket(12345);
    exit(0);
}