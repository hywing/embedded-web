// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved
//
// This example demonstrates how to handle very large requests without keeping
// them in memory.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mongoose.h"

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

struct file_writer_data {
  FILE *fp;
  size_t bytes_written;
};

#ifndef MAX_BUFFER_LENGTH
#define MAX_BUFFER_LENGTH 32
#endif

struct file_list
{
    char *name;
    FILE *file;
    size_t size;
    char type[MAX_BUFFER_LENGTH];
    struct file_list *next;
};

static char s_current_file[MAX_BUFFER_LENGTH] = {0};

static void handle_upload(struct mg_connection *nc, int ev, void *p) {
  struct file_list *data = (struct file_list *) nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

  switch (ev) {
    case MG_EV_HTTP_PART_BEGIN: {
      if (data == NULL) {
        data = calloc(1, sizeof(struct file_list));
        data->name = calloc(strlen(mp->file_name) + 1, sizeof (char));
        sprintf(data->name, "/tmp/%s", mp->file_name);
        data->file = fopen(data->name, "w");
        data->size = 0;
        data->next = NULL;

        memset(s_current_file, 0, sizeof (s_current_file));
        strcpy(s_current_file, mp->file_name);
        memset(data->type, 0, sizeof (data->type));
        strcpy(data->type, mp->var_name);

        if (data->file == NULL) {
          mg_printf(nc, "%s",
                    "HTTP/1.1 500 Failed to open a file\r\n"
                    "Content-Length: 0\r\n\r\n");
          nc->flags |= MG_F_SEND_AND_CLOSE;
          free(data->name);
          free(data);
          return;
        }
        nc->user_data = (void *) data;
      }
      break;
    }
    case MG_EV_HTTP_PART_DATA: {
      if(strcmp(s_current_file, mp->file_name))
      {
          struct file_list *node = calloc(1, sizeof(struct file_list));
          node->name = calloc(strlen(mp->file_name) + 1, sizeof (char));
          sprintf(node->name, "/tmp/%s", mp->file_name);
          node->file = fopen(node->name, "w");
          node->size = 0;
          node->next = NULL;

          memset(s_current_file, 0, sizeof (s_current_file));
          strcpy(s_current_file, mp->file_name);
          memset(node->type, 0, sizeof (node->type));
          strcpy(node->type, mp->var_name);

          struct file_list *p = data;
          while(p->next)
          {
              p = p->next;
          }
          p->next = node;

          if(fwrite(mp->data.p, 1, mp->data.len, node->file) != mp->data.len)
          {
              mg_printf(nc, "%s",
                        "HTTP/1.1 500 Failed to open a file\r\n"
                        "Content-Length: 0\r\n\r\n");
              nc->flags |= MG_F_SEND_AND_CLOSE;
              free(node->name);
              free(node);
              return ;
          }
          node->size += mp->data.len;
      }
      else
      {
          struct file_list *p = data;
          char *name = p->name + 5;
          while(p->next)
          {
              if(!strcmp(s_current_file, name))
              {
                  break;
              }
              p = p->next;
          }

          if(fwrite(mp->data.p, 1, mp->data.len, p->file) != mp->data.len)
          {
              mg_printf(nc, "%s",
                        "HTTP/1.1 500 Failed to open a file\r\n"
                        "Content-Length: 0\r\n\r\n");
              nc->flags |= MG_F_SEND_AND_CLOSE;
              p = data;
              while(p)
              {
                  struct file_list *tmp = p->next;
                  free(p->name);
                  free(p);
                  p = tmp;
              }
              return ;
          }

          p->size += mp->data.len;

          memset(s_current_file, 0, sizeof (s_current_file));
          strcpy(s_current_file, mp->file_name);
          memset(p->type, 0, sizeof (p->type));
          strcpy(p->type, mp->var_name);
      }
      break;
    }
    case MG_EV_HTTP_PART_END: {
      struct file_list *p = data;

      while(p)
      {
          fclose(p->file);
          mg_printf(nc,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Connection: close\r\n\r\n"
                    "File %s has been download sucessfully\n\n",
                    p->name);
          p = p->next;
      }

      nc->flags |= MG_F_SEND_AND_CLOSE;
      p = data;
      while(p)
      {
          struct file_list *tmp = p->next;
          free(p->name);
          free(p);
          p = tmp;
      }
      nc->user_data = NULL;
      break;
    }
  }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_REQUEST) {
    mg_serve_http(nc, ev_data, s_http_server_opts);
  }
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *c;

  mg_mgr_init(&mgr, NULL);
  c = mg_bind(&mgr, s_http_port, ev_handler);
  if (c == NULL) {
    fprintf(stderr, "Cannot start server on port %s\n", s_http_port);
    exit(EXIT_FAILURE);
  }

  s_http_server_opts.document_root = "/mnt/hgfs/workspace/embedded-web/upgrade-server/web-root";  // Serve current directory
  mg_register_http_endpoint(c, "/upload", handle_upload MG_UD_ARG(NULL));

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(c);

  printf("Starting web server on port %s\n", s_http_port);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
