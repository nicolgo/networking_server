#include "lib_gutil.h"
#include "lib_http.h"
#include "buffer.h"
#include "lib_tcp_server.h"

/**************************** http request *****************/

#define INIT_REQUEST_HEADER_SIZE 128
#define INIT_RESPONSE_HEADER_SIZE 128

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
http_response_struc* http_response_init()
{
    http_response_struc* http_response = malloc(sizeof(http_response_struc));
    http_response->body = NULL;
    http_response->status_code = UnKnown;
    http_response->status_message = NULL;
    http_response->response_headers = malloc(sizeof(http_response_header_struc) * INIT_RESPONSE_HEADER_SIZE);
    http_response->response_headers_number = 0;
    http_response->keep_connected = 0;

    return http_response;
}
void http_response_free(http_response_struc* http_response)
{
    if (http_response != NULL) {
        if (http_response->response_headers != NULL) {
            for (int i = 0;i < http_response->response_headers_number;i++) {
                free(http_response->response_headers[i].key);
                free(http_response->response_headers[i].value);
            }
            free(http_response->response_headers);
        }
        free(http_response);
    }
    return;
}
void http_response_encode_buffer(http_response_struc* http_response,
    buffer_struc* output)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", http_response->status_code);
    buffer_append_string(output, buf);
    buffer_append_string(output, http_response->status_message);
    buffer_append_string(output, "\r\n");

    if (http_response->keep_connected) {
        buffer_append_string(output, "Connection: close\r\n");
    }
    else {
        snprintf(buf, sizeof(buf), "Content-Length:%zd\r\n", strlen(http_response->body));
        buffer_append_string(output, buf);
        buffer_append_string(output, "Connection: Keep-Alive\r\n");
    }

    if (http_response->response_headers != NULL
        && http_response->response_headers_number > 0) {
        for (int i = 0;i < http_response->response_headers_number;i++) {
            buffer_append_string(output, http_response->response_headers[i].key);
            buffer_append_string(output, ": ");
            buffer_append_string(output, http_response->response_headers[i].value);
            buffer_append_string(output, "\r\n");

        }
    }
    buffer_append_string(output, "\r\n");
    buffer_append_string(output, http_response->body);
    return;
}

/********************* http server  ****************************************/

static int process_status_line(char* start, char* end,
    http_request_struc* http_request)
{
    int size = end - start;
    //method
    char* space = memmem(start, end - start, " ", 1);
    assert(space != NULL);
    int method_size = space - start;
    http_request->method = malloc(method_size + 1);
    strncpy(http_request->method, start, space - start);
    http_request->method[method_size + 1] = '\0';

    //url
    start = space + 1;
    space = memmem(start, end - start, " ", 1);
    assert(space != NULL);
    int url_size = space - start;
    http_request->url = malloc(url_size + 1);
    strncpy(http_request->url, start, space - start);
    http_request->url[url_size + 1] = '\0';

    //version
    start = space + 1;
    http_request->version = malloc(end - start + 1);
    strncpy(http_request->version, start, end - start);
    http_request->version[end - start + 1] = '\0';
    assert(space != NULL);
    return size;
}

int parse_http_request(buffer_struc* request, http_request_struc* http_request)
{
    int ok = 1;
    // printf("%s",request->data);
    while (http_request->current_state != REQUEST_DONE) {
        if (http_request->current_state == REQUEST_STATUS) {
            char* crlf = buffer_find_CRLF(request);
            if (crlf) {
                int request_line_size = process_status_line(request->data
                    + request->read_index, crlf, http_request);
                if (request_line_size) {
                    request->read_index += request_line_size;
                    request->read_index += 2;
                    // change the state
                    http_request->current_state = REQUEST_HEADERS;
                }
            }
        }
        else if (http_request->current_state == REQUEST_HEADERS) {
            char* crlf = buffer_find_CRLF(request);
            if (crlf) {
                char *start = request->data + request->read_index;
                int request_line_size = crlf - start;
                char *colon = memmem(start, request_line_size, ": ", 2);
                if (colon != NULL) {
                    char *key = malloc(colon - start + 1);
                    strncpy(key, start, colon - start);
                    key[colon - start] = '\0';
                    char *value = malloc(crlf - colon - 2 + 1);
                    strncpy(value, colon + 2, crlf - colon - 2);
                    value[crlf - colon - 2] = '\0';

                    http_request_add_header(http_request, key, value);

                    request->read_index += request_line_size; //request line size
                    request->read_index += 2;  //CRLF size
                } else {
                    //The last line.
                    request->read_index += 2;  //CRLF size
                    http_request->current_state = REQUEST_DONE;
                }
            }
        }
    }
    return ok;
}

static int http_on_message(buffer_struc* input, tcp_connection_struc* tcp_connection)
{
    net_msgx("message from tcp connection %s", tcp_connection->name);
    http_request_struc* http_request = (http_request_struc*)tcp_connection->request;
    http_server_struc* http_server = (http_server_struc*)tcp_connection->data;

    if (parse_http_request(input, http_request) == 0) {
        char* error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        tcp_connection_send_data(tcp_connection, error_response, sizeof(error_response));
        tcp_connection_shutdown(tcp_connection);
    }

    if (http_request_current_state(http_request) == REQUEST_DONE) {
        http_response_struc* http_response = http_response_init();
        if (http_server->request_callback != NULL) {
            http_server->request_callback(http_request, http_response);
        }

        buffer_struc* buffer = buffer_new();
        http_response_encode_buffer(http_response, buffer);
        tcp_connection_send_buffer(tcp_connection, buffer);

        if (http_request_close_connection(http_request)) {
            tcp_connection_shutdown(tcp_connection);
        }
        http_request_reset(http_request);
    }

    return 0;
}

static int http_on_connection_completed(tcp_connection_struc* tcp_connection)
{
    net_msgx("connection conpleted");
    http_request_struc* http_request = http_request_init();
    tcp_connection->request = http_request;
    return 0;
}

static int http_on_write_completed(tcp_connection_struc* tcp_connection)
{
    net_msgx("write completed");
    return 0;
}

static int http_on_connect_closed(tcp_connection_struc* tcp_connection)
{
    net_msgx("connection closed");
    if (tcp_connection->request != NULL) {
        http_request_free(tcp_connection->request);
        tcp_connection->request = NULL;
    }
    return 0;
}

http_server_struc* http_server_init(event_loop_struc* event_loop,
    int port, request_callback_f request_callback, int thread_num)
{
    socket_acceptor_struct* acceptor = socket_acceptor_init(SERVER_PORT);
    http_server_struc* http_server = malloc(sizeof(http_server_struc));
    http_server->request_callback = request_callback;

    http_server->tcp_server = tcp_server_init(event_loop, acceptor,
        http_on_connection_completed,
        http_on_message,
        http_on_write_completed,
        http_on_connect_closed, thread_num);

    http_server->tcp_server->data = http_server;

    return http_server;

}
void http_server_start(http_server_struc* http_server)
{
    tcp_server_start(http_server->tcp_server);
}
