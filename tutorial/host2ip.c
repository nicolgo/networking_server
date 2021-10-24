#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *res,*tmp;
    char host[256];

    if(argc != 2){
        fprintf(stderr,"usage: %s hostname\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_INET;//IPv4

    int ret = getaddrinfo(argv[1],NULL,&hints,&res);
    if(ret != 0){
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    for(tmp=res;tmp!=NULL;tmp=tmp->ai_next){
        getnameinfo(tmp->ai_addr,tmp->ai_addrlen,host,sizeof(host),NULL,0,NI_NUMERICHOST);
        puts(host);
    }
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}