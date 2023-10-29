#include "mongoose.h"

static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_READ) {
        printf("from client msg : %s\n", c->recv.buf);
        mg_send(c, c->recv.buf, c->recv.len);     // Echo received data back
        mg_iobuf_del(&c->recv, 0, c->recv.len);   // And discard it
    }
    else if(ev == MG_EV_CONNECT) {
        printf("Connection established\n");
    }
    else if(ev == MG_EV_CLOSE) {
        printf("Connection closed\n");
    }
    else if(ev == MG_EV_OPEN) {
        printf("Connection created\n");
    }
    else if(ev == MG_EV_ACCEPT) {
        printf("Connection accepted\n");
    }
}

int main(int argc, char *argv[])
{
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);                                  // Init manager
    mg_listen(&mgr, "tcp://0.0.0.0:1234", cb, &mgr);    // Setup listener
    for (;;) mg_mgr_poll(&mgr, 1000);                   // Event loop
    mg_mgr_free(&mgr);                                  // Cleanup
    return 0;
}
