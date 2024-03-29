#include "mongoose.h"

static const char *s_url = "mqtt://119.23.145.144:1883";   // MQTT URL
static const char *s_sub_topic = "/iotstuff";               // Publish topic
static const char *s_pub_topic = "mg/clnt/test";           // Subscribe topic
static int s_qos = 1;                                      // MQTT QoS
static struct mg_connection *s_conn;                       // Client connection

// Handle interrupts, like Ctrl-C
static int s_signo;
static void signal_handler(int signo) {
    s_signo = signo;
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_OPEN) {
        MG_INFO(("%lu CREATED", c->id));
        // c->is_hexdumping = 1;
    } else if (ev == MG_EV_ERROR) {
        // On error, log error message
        MG_ERROR(("%lu ERROR %s", c->id, (char *) ev_data));
    } else if (ev == MG_EV_MQTT_OPEN) {
        // MQTT connect is successful
        struct mg_str subt = mg_str(s_sub_topic);
        struct mg_str pubt = mg_str(s_pub_topic), data = mg_str("hello");
        MG_INFO(("%lu CONNECTED to %s", c->id, s_url));
        struct mg_mqtt_opts sub_opts;
        memset(&sub_opts, 0, sizeof(sub_opts));
        sub_opts.topic = subt;
        sub_opts.qos = s_qos;
        mg_mqtt_sub(c, &sub_opts);
        MG_INFO(("%lu SUBSCRIBED to %.*s", c->id, (int) subt.len, subt.ptr));
        struct mg_mqtt_opts pub_opts;
        memset(&pub_opts, 0, sizeof(pub_opts));
        pub_opts.topic = pubt;
        pub_opts.message = data;
        pub_opts.qos = s_qos, pub_opts.retain = false;
        mg_mqtt_pub(c, &pub_opts);
        MG_INFO(("%lu PUBLISHED %.*s -> %.*s", c->id, (int) data.len, data.ptr,
                 (int) pubt.len, pubt.ptr));
    } else if (ev == MG_EV_MQTT_MSG) {
        // When we get echo response, print it
        struct mg_mqtt_message *mm = (struct mg_mqtt_message *) ev_data;
        MG_INFO(("%lu RECEIVED %.*s <- %.*s", c->id, (int) mm->data.len,
                 mm->data.ptr, (int) mm->topic.len, mm->topic.ptr));
    } else if (ev == MG_EV_CLOSE) {
        MG_INFO(("%lu CLOSED", c->id));
        s_conn = NULL;  // Mark that we're closed
    }
    (void) fn_data;
}

// Timer function - recreate client connection if it is closed
static void timer_fn(void *arg) {
    struct mg_mgr *mgr = (struct mg_mgr *) arg;
    struct mg_mqtt_opts opts = {.clean = true,
                .qos = s_qos,
                .topic = mg_str(s_pub_topic),
                .version = 4,
                .message = mg_str("bye")};
    if (s_conn == NULL) s_conn = mg_mqtt_connect(mgr, s_url, &opts, fn, NULL);
//    else
//    {
//        struct mg_str pubt = mg_str(s_pub_topic), data = mg_str("hello");
//        struct mg_mqtt_opts pub_opts;
//        memset(&pub_opts, 0, sizeof(pub_opts));
//        pub_opts.topic = pubt;
//        pub_opts.message = data;
//        pub_opts.qos = 1;
//        mg_mqtt_pub(s_conn, &pub_opts);
//    }
}

int main(int argc, char *argv[]) {
    struct mg_mgr mgr;
    int i;

    // Parse command-line flags
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && argv[i + 1] != NULL) {
            s_url = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && argv[i + 1] != NULL) {
            s_pub_topic = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && argv[i + 1] != NULL) {
            s_sub_topic = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0 && argv[i + 1] != NULL) {
            mg_log_set(atoi(argv[++i]));
        } else {
            MG_ERROR(("Unknown option: %s. Usage:", argv[i]));
            MG_ERROR(
                        ("%s [-u mqtts://SERVER:PORT] [-p PUB_TOPIC] [-s SUB_TOPIC] "
                         "[-v DEBUG_LEVEL]",
                         argv[0], argv[i]));
            return 1;
        }
    }

    signal(SIGINT, signal_handler);   // Setup signal handlers - exist event
    signal(SIGTERM, signal_handler);  // manager loop on SIGINT and SIGTERM

    mg_mgr_init(&mgr);
    struct mg_tls_opts opts = {.client_ca = mg_str(CA_ALL)};
    mg_tls_ctx_init(&mgr, &opts);
    mg_timer_add(&mgr, 3000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, timer_fn, &mgr);
    while (s_signo == 0) mg_mgr_poll(&mgr, 1000);  // Event loop, 1s timeout
    mg_mgr_free(&mgr);                             // Finished, cleanup

    return 0;
}
