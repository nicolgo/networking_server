#include "lib_gutil.h"

/************* http request *****************/
http_request_struc *http_request_init()
{
    http_request_struc *http_request = malloc(sizeof(http_request_struc));

}
void http_request_clear(http_request_struc* http_request);
void http_request_reset(http_request_struc* http_request);
void http_request_add_header(http_request_struc* http_request,char* key,char* value);
char *http_request_get_header(http_request_struc* http_request,char* key);
enum http_request_state http_request_current_state(http_request_struc* http_request);
int http_request_close_connection(http_request_struc* http_request);

/********************* http response *****************************/


/********************* http server  ****************************************/
http_server_struc* http_server_init(event_loop_struc* event_loop,
    int port, request_callback_f request_callback, int thread_num)
{
    socket_acceptor_struct *acceptor = socket_acceptor_init(SERVER_PORT);
    http_server_struc *http_server = malloc(sizeof(http_server_struc));
    http_server->request_callback = request_callback;
    


}
void http_server_start(http_server_struc *http_server)
{
    tcp_server_start(http_server->tcp_server);
}
int parse_http_request(buffer_struc *request,http_request_struc *http_request);