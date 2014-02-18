
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* for uint32_t */
#include <stdint.h>

#include "bitfield.h"
#include "onefolder.h"
#include "onefolder_connection.h"
#include "onefolder_msghandler.h"

typedef struct {
    void* udata;
} conn_private_t;

of_conn_t* of_conn_new(of_conn_cb_t* cb, void* udata)
{
    conn_private_t *me;

    me = calloc(1, sizeof(conn_private_t));
    me->udata = udata;

    return (of_conn_t*)me;
}

#if 0
void of_conn_keepalive(of_conn_t* pco)
{
#if 0
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_KEEPALIVE;
#endif
}

void of_conn_fulllog(of_conn_t* pco, msg_fulllog_t *m)
{
#if 0
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_FULLLOG;
    memcpy(&pc->fulllog, m, sizeof(msg_fulllog_t));
#endif
}

void of_conn_pwp(of_conn_t* pco, msg_pwp_t *m)
{
#if 0
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_FULLLOG;
    memcpy(&pc->fulllog, m, sizeof(msg_pwp_t));
#endif
}

static int of_conn_pwp_dispatch(void *pc_,
        const unsigned char* buf, unsigned int len)
{
//    uv_mutex_lock(&me->mutex);
//    bt_dm_dispatch_from_buffer(me->bc,peer_nethandle,buf,len);
//    uv_mutex_unlock(&me->mutex);
    return 0;
}
#endif

#if 0
void of_conn_filelog(of_conn_t* pco, char* filelog, int len)
{
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_FULLLOG;
    memcpy(&pc->fulllog, m, sizeof(msg_fulllog_t));
}
#endif

#if 0
void of_conn_piecelog(of_conn_t* pco, char* filelog, int len)
{
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_FULLLOG;
    memcpy(&pc->fulllog, m, sizeof(msg_fulllog_t));
}
#endif

