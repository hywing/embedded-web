// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include "mongoose.h"
#include "cJSON.h"

#ifndef OK
#define OK 1
#endif

#ifndef ERR
#define ERR 0
#endif

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static void on_data_come(struct mg_connection *nc, struct websocket_message *wm)
{
    char data[1024] = {0};
    snprintf(data, wm->size + 1, "%s", wm->data);

    cJSON *json = cJSON_Parse(data);
    cJSON *item = NULL;
    if((item = cJSON_GetObjectItem(json, "user")))
    {
        sprintf(data, "{\"item\":\"user\",\"result\":%d}", !strlen(item->valuestring) ? ERR : OK);
    }
    else if((item = cJSON_GetObjectItem(json, "number")))
    {
        sprintf(data, "{\"item\":\"number\",\"result\":%d}", !strlen(item->valuestring) ? ERR : OK);
    }
    else if((item = cJSON_GetObjectItem(json, "sex")))
    {
        sprintf(data, "{\"item\":\"sex\",\"result\":%d}", !strlen(item->valuestring) ? ERR : OK);
    }
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, data, strlen(data));
}

static void ev_handler(struct mg_connection *nc, int ev, void *p)
{
    if (ev == MG_EV_HTTP_REQUEST)
    {
        mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
    }
    else if(ev == MG_EV_WEBSOCKET_FRAME)
    {

        on_data_come(nc, (struct websocket_message *)p);
    }
}

int main(void)
{
    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);
    printf("Starting web server on port %s\n", s_http_port);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    if (nc == NULL) {
        printf("Failed to create listener\n");
        return 1;
    }

    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = "/mnt/hgfs/workspace/embedded-web/tiny-webserver/web-root";
    s_http_server_opts.enable_directory_listing = "yes";

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}
