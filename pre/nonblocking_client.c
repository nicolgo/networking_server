#include "lib_gutil.h"

int main()
{
    int sockfd = tcp_client(NULL, "12345");

    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 0;
    // when socket close, send RST immediately. 
    setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    close(sockfd);
    exit(EXIT_SUCCESS);
}