
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

