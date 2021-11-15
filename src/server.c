#define _GNU_SOURCE
#include "lib_gutil.h"
#include <string.h>
#define PORT 21042

int handle_request(http_request_struc* http_request,
    http_response_struc* http_response) {
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
        http_response->status_code = OK;
        http_response->status_message = "OK";
        http_response->content_type = "text/html";
        http_response->body = "<html><head><title> \
        COMP 5311</title></head><body><h1>Internet Infrastructure and Ptotocols</h1></body></html>";
    }
    else if (strcmp(path, "/network") == 0) {
        http_response->status_code = OK;
        http_response->status_message = "OK";
        http_response->content_type = "text/plain";
        http_response->body = "COMP 5311";
    }
    else {
        http_response->status_code = NotFound;
        http_response->status_message = "Not Found";
        http_response->keep_connected = 1;
    }

    return 0;
}

int test_http_server()
{
    char *data = "GET / HTTP/1.1\r\nHost: localhost:21042\r\nUser-Agent: curl/7.54.0\r\nAccept: */*\r\n\r\n";
    buffer_struc* buffer = buffer_new();
    buffer_append_string(buffer,data);
    http_request_struc *http_request = http_request_init();
    parse_http_request(buffer,http_request);
    
    for(int i = 0;i <http_request->request_headers_number;i++){
        char* key = http_request->request_headers[i].key;
        char* value = http_request->request_headers[i].value;
        printf("key == %s, value == %s\n",key,value);
    }

    http_response_struc *http_response = http_response_init();
    handle_request(http_request,http_response);

    buffer_struc *output = buffer_new();
    http_response_encode_buffer(http_response,output);

    buffer_free(buffer);
    buffer_free(output);
    http_request_free(http_request);
    http_response_free(http_response);
    return 0;
}

int main(void)
{

    //test_http_server();
    char *thread_name = NULL;
    event_loop_struc* event_loop = event_loop_init(thread_name);

    http_server_struc* http_server = http_server_init(event_loop, PORT, handle_request, 2);
    http_server_start(http_server);

    event_loop_run(event_loop);

    if (event_loop != NULL) {
        if (event_loop->channel_map != NULL) {
            free(event_loop->channel_map);
        }
        free(event_loop);
        event_loop = NULL;
    }

    exit(EXIT_SUCCESS);
}