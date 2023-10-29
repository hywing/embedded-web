#include "mongoose.h"

static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_READ) {
        printf("from server msg : %s\n", c->recv.buf);
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

static void timer_fn(void *arg) {
    struct mg_mgr *mgr = (struct mg_mgr *) arg;
    char *buf = "hello world";
    mg_send(mgr->conns, (const void *)buf, 12);
}

int main(int argc, char *argv[])
{
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);                                                              // Init manager
    mg_connect(&mgr, "tcp://localhost:1234", cb, &mgr);                             // Connect host
    mg_timer_add(&mgr, 1000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, timer_fn, &mgr);   // Setup timer
    for (;;) mg_mgr_poll(&mgr, 1000);                                               // Event loop
    mg_mgr_free(&mgr);                                                              // Cleanup
    return 0;
}
