#define _GNU_SOURCE
#include "lib_gutil.h"
#include <string.h>
#define PORT 21042

int act_for_request(http_request_struc* http_request,
    http_request_struc* http_response) {
    char* url = http_request->url;
    char* question = memmem(url, strlen(url), "?", 1);
    char* path = NULL;
    if (question != NULL) {
        path = malloc(question - url);
        strncpy(path, url, question - url);
    }
    else {
        path = malloc(strlen(url));
        strncpy(path, url, strlen(url));
    }

    if (strcmp(path, "/") == 0) {
        
    }
    if (path != NULL) {
        free(path);
    }
    return 0;
}

int main(int c, char* argv[])
{
    event_loop_struc* event_loop = event_loop_init(NULL);

    http_server_struc* http_server = http_server_init(event_loop, PORT, act_for_request, 2);
    http_server_start(http_server);

    event_loop_run(event_loop);

    if(event_loop != NULL){
        if(event_loop->channel_map != NULL){
            free(event_loop->channel_map);
        }
        free(event_loop);
        event_loop = NULL;
    }

    exit(EXIT_SUCCESS);
}