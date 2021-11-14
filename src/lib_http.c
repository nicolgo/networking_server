#include "lib_gutil.h"

/************* http request *****************/

#define INIT_REQUEST_HEADER_SIZE 128

const char* HTTP10 = "HTTP/1.0";
const char* HTTP11 = "HTTP/1.1";
const char* KEEP_ALIVE = "Keep-Alive";
const char* CLOSE = "close";

http_request_struc* http_request_init()
{
    http_request_struc* http_request = malloc(sizeof(http_request_struc));
    http_request->method = NULL;
    http_request->current_state = REQUEST_STATUS;
    http_request->version = NULL;
    http_request->url = NULL;
    http_request->request_headers = malloc(sizeof(http_request_struc) * INIT_REQUEST_HEADER_SIZE);
    http_request->request_headers_number = 0;
    return http_request;

}
void http_request_free(http_request_struc* http_request)
{
    if (http_request != NULL) {
        if (http_request->request_headers != NULL) {
            for (int i = 0;i < http_request->request_headers_number;i++) {
                free(http_request->request_headers[i].key);
                free(http_request->request_headers[i].value);
            }
            free(http_request->request_headers);
        }
        free(http_request);
    }
}
void http_request_reset(http_request_struc* http_request)
{
    http_request->method = NULL;
    http_request->current_state = REQUEST_STATUS;
    http_request->version = NULL;
    http_request->url = NULL;
    http_request->request_headers_number = 0;
}
void http_request_add_header(http_request_struc* http_request, char* key, char* value)
{
    http_request->request_headers[http_request->request_headers_number].key = key;
    http_request->request_headers[http_request->request_headers_number].value = value;
    http_request->request_headers_number++;
}
char* http_request_get_header(http_request_struc* http_request, char* key)
{
    if (http_request->request_headers != NULL) {
        for (int i = 0;i < http_request->request_headers_number;i++) {
            if (strncmp(http_request->request_headers[i].key, key, strlen(key)) == 0) {
                return http_request->request_headers[i].value;
            }
        }
    }
    return NULL;
}
enum http_request_state http_request_current_state(http_request_struc* http_request)
{
    return http_request->current_state;
}
int http_request_close_connection(http_request_struc* http_request)
{
    char* connection = http_request_get_header(http_request, "Connection");

    if (connection != NULL && strncmp(connection, CLOSE, strlen(CLOSE)) == 0) {
        return 1;
    }

    if (http_request->version != NULL
        && strncmp(http_request->version, HTTP10, strlen(HTTP10)) == 0
        && strncmp(connection, KEEP_ALIVE, strlen(KEEP_ALIVE)) == 1) {
        return 1;
    }
    return 0;
}

/********************* http response *****************************/


/********************* http server  ****************************************/
http_server_struc* http_server_init(event_loop_struc* event_loop,
    int port, request_callback_f request_callback, int thread_num)
{
    socket_acceptor_struct* acceptor = socket_acceptor_init(SERVER_PORT);
    http_server_struc* http_server = malloc(sizeof(http_server_struc));
    http_server->request_callback = request_callback;



}
void http_server_start(http_server_struc* http_server)
{
    tcp_server_start(http_server->tcp_server);
}
int parse_http_request(buffer_struc* request, http_request_struc* http_request);