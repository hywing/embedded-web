#include "mongoose.h"

#ifndef OK
#define OK 1
#endif

#ifndef ERR
#define ERR 0
#endif

#ifndef INTERVAL
#define INTERVAL 1
#endif

#ifndef MAX_BUFFER_LEN
#define MAX_BUFFER_LEN 1024
#endif

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
static struct mg_mgr s_mgr;
static struct mg_connection *s_newest = NULL;

static void send_log_data(struct mg_connection *nc, const char *str)
{
    char data[MAX_BUFFER_LEN] = {0};
    sprintf(data, "{\"log\":%s}", str);
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, data, strlen(data));
}

static void web_handler(struct mg_connection *nc, int ev, void *p)
{
    switch (ev)
    {
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
    {
        s_newest = nc;
        printf("new connection %p join++\n", nc);
        break;
    }
    case MG_EV_HTTP_REQUEST:
    {
        mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
        break;
    }
    case MG_EV_TIMER:
    {
        mg_set_timer(nc, mg_time() + INTERVAL);
        if(s_newest)
        {
            srand(time(NULL));
            char str[MAX_BUFFER_LEN] = {0};
            sprintf(str, "%d", rand() % 10000);
            send_log_data(s_newest, str);
        }
        break;
    }
    case MG_EV_CLOSE:
    {
        printf("new connection %p left--\n", nc);
        break;
    }
    default:
        break;
    }
}

int main(void)
{
    struct mg_connection *nc;

    // init http server
    mg_mgr_init(&s_mgr, NULL);
    printf("Starting web server on port %s\n", s_http_port);
    nc = mg_bind(&s_mgr, s_http_port, web_handler);
    if (nc == NULL) {
        printf("Failed to create listener\n");
        return 1;
    }

    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = "/mnt/hgfs/workspace/embedded-web/log-server/web-root";
    s_http_server_opts.enable_directory_listing = "yes";

    // init timer
    mg_set_timer(nc, mg_time() + INTERVAL);

    for (;;) {
        mg_mgr_poll(&s_mgr, 1000);
    }
    mg_mgr_free(&s_mgr);

    return 0;
}
