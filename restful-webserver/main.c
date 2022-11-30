/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "mongoose.h"

#ifndef MAX_BUFFER_LEN
#define MAX_BUFFER_LEN 1024
#endif

static const char *s_http_port = "8000";
static char s_version[MAX_BUFFER_LEN] = "V1.0.0.0";

static void restful_handler(struct mg_connection *nc, struct http_message *hm)
{
    if(mg_vcmp(&hm->method, "GET") == 0)
    {
        if(mg_vcmp(&hm->uri, "/version") == 0)
        {
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_printf_http_chunk(nc, "{ \"version\": \"%s\" }", s_version);
            mg_send_http_chunk(nc, "", 0);
        }
    }
    else if(mg_vcmp(&hm->method, "PUT") == 0)
    {
        memset(s_version, 0, sizeof(s_version));
        if(mg_vcmp(&hm->uri, "/version") == 0)
        {
            snprintf(s_version, hm->body.len + 1, "%s", hm->body.p);
            printf("version : %s\n", s_version);
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_printf_http_chunk(nc, "{ \"result\": %d }", 1);
            mg_send_http_chunk(nc, "", 0);
        }

    }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    struct http_message *hm = (struct http_message *) ev_data;
    switch (ev) {
    case MG_EV_HTTP_REQUEST:
    {
        restful_handler(nc, hm);
        break;
    }
    default:
        break;
    }
}

int main(void)
{
  struct mg_mgr mgr;
  struct mg_connection *nc = NULL;

  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, s_http_port, ev_handler);
  mg_set_protocol_http_websocket(nc);

  printf("Starting restful server on port %s \n", s_http_port);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
