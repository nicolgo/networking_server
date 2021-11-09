#ifndef LIB_HTTP_H
#define LIB_HTTP_H

#include "lib_gutil.h"


typedef struct http_request_header_struc {
    char* key;
    char* value;
}http_request_header_struc;

enum http_request_state {
    REQUEST_STATUS,    //wait parse status
    REQUEST_HEADERS,   //wait parse headers
    REQUEST_BODY,      //wait parse body
    REQUEST_DONE       //parse finished
};

typedef struct http_request_struc {
    char* version;
    char* method;
    char* url;
    enum http_request_state current_state;
    struct http_request_header_struc* request_headers;
    int request_headers_number;
}http_request_struc;

typedef struct http_response_struct {

}http_response_struct;


typedef int (*request_callback_f)(http_request_struc* http_request,
    http_response_struct* http_response);

typedef struct http_server_struc {
    request_callback_f request_callback;
}http_server_struc;

http_server_struc* http_server_init(event_loop_struc* event_loop,
    int port, request_callback_f request_callback, int thread_num);

#endif