#include "lib_gutil.h"
#define PORT 21042

int act_for_request(http_request_struc *http_request,
http_request_struc *http_response){
    
    return 0;
}

int main(int c,char* argv[])
{
    event_loop_struc *event_loop = event_loop_init(NULL);

    http_server_struc *http_server = http_server_init(event_loop,PORT,act_for_request,2);
    http_server_start(http_server);

    event_loop_run(event_loop);
    
    exit(EXIT_SUCCESS);
}