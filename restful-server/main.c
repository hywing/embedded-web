/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "mongoose.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#ifndef MAX_BUFFER_LEN
#define MAX_BUFFER_LEN 1024
#endif

#ifndef OK
#define OK 1
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE "./config.json"
#endif

static const char *s_http_port = "8000";
static cJSON *s_json = NULL;

static cJSON *read_json(const char *path)
{
    FILE *fp = fopen(path, "r");
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    char *content = (char *)malloc(size * sizeof(char));
    memset(content, 0, size);
    fseek(fp, 0, SEEK_SET);
    fread(content, 1, size, fp);
    fclose(fp);
    cJSON *json = cJSON_Parse(content);
    free(content);
    return json;
}

static void write_json(const char *path, cJSON *json)
{
    char *out = cJSON_Print(json);
    int len = 0;
    len = strlen(out);
    FILE *fp = fopen(path, "w");
    if(NULL == fp)
    {
        printf("write %s failed\n", path);
        return;
    }
    fwrite(out, len, 1, fp);
    fclose(fp);
    out = NULL;
}

static void restful_handler(struct mg_connection *nc, struct http_message *hm)
{
    if(mg_vcmp(&hm->method, "GET") == 0)
    {
        if(mg_vcmp(&hm->uri, "/version") == 0)
        {
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            cJSON *item = cJSON_GetObjectItem(s_json, "version");
            printf("json : %s\n", cJSON_Print(item));
            mg_printf_http_chunk(nc, "{ \"version\": \"%s\" }", item->valuestring);
            mg_send_http_chunk(nc, "", 0);
        }
        else if(mg_vcmp(&hm->uri, "/author") == 0)
        {
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            cJSON *item = cJSON_GetObjectItem(s_json, "author");
            mg_printf_http_chunk(nc, "{ \"author\": \"%s\" }", item->valuestring);
            mg_send_http_chunk(nc, "", 0);
        }
        else if(mg_vcmp(&hm->uri, "/date") == 0)
        {
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            cJSON *item = cJSON_GetObjectItem(s_json, "date");
            mg_printf_http_chunk(nc, "{ \"date\": \"%s\" }", item->valuestring);
            mg_send_http_chunk(nc, "", 0);
        }
    }
    else if(mg_vcmp(&hm->method, "PUT") == 0)
    {
        if(mg_vcmp(&hm->uri, "/version") == 0)
        {
            cJSON *item = cJSON_GetObjectItem(s_json, "version");
            strncpy(item->valuestring, hm->body.p, hm->body.len);
            write_json(CONFIG_FILE, s_json);
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_printf_http_chunk(nc, "{ \"result\": %d }", OK);
            mg_send_http_chunk(nc, "", 0);
        }
        else if(mg_vcmp(&hm->uri, "/author") == 0)
        {
            cJSON *item = cJSON_GetObjectItem(s_json, "author");
            strncpy(item->valuestring, hm->body.p, hm->body.len);
            write_json(CONFIG_FILE, s_json);
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_printf_http_chunk(nc, "{ \"result\": %d }", OK);
            mg_send_http_chunk(nc, "", 0);
        }
        else if(mg_vcmp(&hm->uri, "/name") == 0)
        {
            cJSON *item = cJSON_GetObjectItem(s_json, "name");
            strncpy(item->valuestring, hm->body.p, hm->body.len);
            write_json(CONFIG_FILE, s_json);
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_printf_http_chunk(nc, "{ \"result\": %d }", OK);
            mg_send_http_chunk(nc, "", 0);
        }
        else if(mg_vcmp(&hm->uri, "/count") == 0)
        {
            cJSON *item = cJSON_GetObjectItem(s_json, "count");
            char buffer[MAX_BUFFER_LEN];
            memset(buffer, 0, sizeof(buffer));
            snprintf(buffer, hm->body.len + 1, "%s", hm->body.p);
            item->valuedouble = atof(buffer);
            write_json(CONFIG_FILE, s_json);
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_printf_http_chunk(nc, "{ \"result\": %d }", OK);
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
    // init server
    struct mg_mgr mgr;
    struct mg_connection *nc = NULL;

    mg_mgr_init(&mgr, NULL);

    nc = mg_bind(&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket(nc);

    printf("Starting restful server on port %s \n", s_http_port);

    // init config
    s_json = read_json(CONFIG_FILE);

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}
