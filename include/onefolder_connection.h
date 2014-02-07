
typedef void* of_conn_t;

typedef struct {
    void (*conn_keepalive)(void* pco);
    void (*conn_fulllog)(void* pco, msg_fulllog_t *m);
    int (*conn_pwp_dispatch)(void *mh,
            const unsigned char* buf,
            unsigned int len);
//    void (*conn_pwp)(of_conn_t* pco, msg_pwp_t *m);
} of_conn_recv_cb_t;

void *pwp_conn_new();

void of_conn_keepalive(of_conn_t* pco);

void of_conn_fulllog(of_conn_t* pco, msg_fulllog_t *m);

void of_conn_pwp(of_conn_t* pco, msg_pwp_t *m);

