#ifndef DOGEBOX_CONNECTION_H
#define DOGEBOX_CONNECTION_H

typedef void* of_conn_t;

typedef struct {
    void (*conn_keepalive)(void* pco);
    void (*conn_fulllog)(void* pco, msg_fulllog_t *m);
    int (*conn_pwp_dispatch)(void *mh,
            const unsigned char* buf,
            unsigned int len);
//    void (*conn_pwp)(of_conn_t* pco, msg_pwp_t *m);
} of_conn_recv_cb_t;

typedef struct {
    int (*conn_pwp_dispatch)(void *mh,
            const unsigned char* buf,
            unsigned int len);
} of_conn_cb_t;

/**
 * @param udata User data that we pass on each callback */
of_conn_t *of_conn_new(of_conn_cb_t* cb, void* udata);

void of_conn_filelog(void* pc, const unsigned char* filelog, unsigned int len);

void of_conn_set_piece_mapper(of_conn_t* me_, void* pm);

void of_conn_set_piece_db(of_conn_t* me_, void* db);

/**
 * @return 0 on failure */
int of_conn_filelog(void* pc, const unsigned char* buf, unsigned int len);

/**
 * @return 0 on failure */
int of_conn_piecelog(void* pc, const unsigned char* buf, unsigned int len);

#endif /* DOGEBOX_CONNECTION_H */
