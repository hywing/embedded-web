/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "mongoose.h"

/* RESTful server host and request URI */
static const char *s_url = "http://192.168.52.150:8000";

static int s_exit_flag = 0;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
  struct http_message *hm = (struct http_message *) ev_data;
  int connect_status;

  switch (ev)
  {
    case MG_EV_CONNECT:
      connect_status = *(int *) ev_data;
      if (connect_status != 0)
      {
        printf("Error connecting to %s: %s\n", s_url, strerror(connect_status));
        s_exit_flag = 1;
      }
      break;
    case MG_EV_HTTP_REPLY:
      printf("Got reply:\n%.*s\n", (int) hm->body.len, hm->body.p);
      nc->flags |= MG_F_SEND_AND_CLOSE;
      s_exit_flag = 1;
      break;
    case MG_EV_CLOSE:
      if (s_exit_flag == 0)
      {
        printf("Server closed connection\n");
        s_exit_flag = 1;
      };
      break;
    default:
      break;
  }
}

static void send_put_request(struct mg_connection *nc, const char *file_name, const char *file_data, const char *host, const char *bucket)
{
    const char *content_type = "text/plain", *method = "PUT";
    char date[100], req[1000];
    time_t now = time(NULL);

    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
    mg_set_protocol_http_websocket(nc);

    snprintf(req, sizeof(req),
             "%s /%s HTTP/1.1\r\n"
             "Host: %s.%s\r\n"
             "Date: %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %lu\r\n"
             "\r\n",
             method, file_name, bucket, host, date, content_type,
             (unsigned long)strlen(file_data));
    mg_printf(nc, "%s%s", req, file_data);
}

int main(void)
{
  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_connect_http(&mgr, ev_handler, s_url, NULL, NULL);
  mg_set_protocol_http_websocket(nc);

  // put request like this : "version : V3.1.4.0"
  send_put_request(nc, "version", "V3.1.4.0", "192.168.101.101", "8000");

  printf("Starting RESTful client against %s\n", s_url);
  while (s_exit_flag == 0) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
