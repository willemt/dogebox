
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "CuTest.h"

#include "bitfield.h"
#include "bitstream.h"
#include "onefolder.h"
#include "onefolder_connection.h"
#include "onefolder_msghandler.h"

typedef struct {
    int payload_len;
    char* payload;
} pwp_t;

typedef struct {

    int mtype;

    union {
        msg_fulllog_t fulllog;
        pwp_t pwp;
    };

} fake_pc_t;

/**
 * Flip endianess
 **/
static uint32_t fe(uint32_t i)
{
    uint32_t o;
    unsigned char *c = (unsigned char *)&i;
    unsigned char *p = (unsigned char *)&o;

    p[0] = c[3];
    p[1] = c[2];
    p[2] = c[1];
    p[3] = c[0];

    return o;
}

void __conn_keepalive(void* pco)
{
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_KEEPALIVE;
}

void __conn_fulllog(void* pco, msg_fulllog_t *m)
{
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_FULLLOG;
    memcpy(&pc->fulllog, m, sizeof(msg_fulllog_t));
}

#if 0
void __conn_pwp(of_conn_t* pco, msg_pwp_t *m)
{
    fake_pc_t* pc = (void*)pco;
    pc->mtype = OF_MSGTYPE_FULLLOG;
    memcpy(&pc->fulllog, m, sizeof(msg_pwp_t));
}
#endif

int __conn_pwp_dispatch(void *pco, const unsigned char* buf, unsigned int len)
{
    fake_pc_t* p = (void*)pco;
    p->mtype = OF_MSGTYPE_PWP;
    p->pwp.payload_len = len;
    p->pwp.payload = malloc(len);
    memcpy(p->pwp.payload, buf, len);
}

void Testof_keepalive(
    CuTest * tc
)
{
    fake_pc_t pc;
    unsigned char data[100];
    unsigned char* ptr;
    void* mh;

    /* keepalive */
    ptr = data;
    mh = of_msghandler_new(&pc, &((of_conn_recv_cb_t){
            .conn_keepalive = __conn_keepalive,
            .conn_fulllog = __conn_fulllog,
            .conn_pwp_dispatch = __conn_pwp_dispatch}));
    bitstream_write_ubyte(&ptr, OF_MSGTYPE_KEEPALIVE);
    CuAssertTrue(tc, 1 == of_msghandler_dispatch_from_buffer(mh, data, 1));
    CuAssertTrue(tc, OF_MSGTYPE_KEEPALIVE == pc.mtype);
    of_msghandler_release(mh);
}

void Testof_fulllog(
    CuTest * tc
)
{
    fake_pc_t pc;
    unsigned char data[100];
    unsigned char* ptr;
    void* mh;

    ptr = data;
    mh = of_msghandler_new(&pc, &((of_conn_recv_cb_t){
            .conn_keepalive = __conn_keepalive,
            .conn_fulllog = __conn_fulllog,
            .conn_pwp_dispatch = __conn_pwp_dispatch}));
    bitstream_write_ubyte(&ptr, OF_MSGTYPE_FULLLOG);
    bitstream_write_uint32(&ptr, fe(16));
    bitstream_write_uint32(&ptr, fe(12));
    /* contents is irrelevant for this test */
    bitstream_write_string(&ptr, "ld4:path4:abcdee", 16);
    bitstream_write_string(&ptr, "ld3:idxi1eee", 12);
    of_msghandler_dispatch_from_buffer(mh, data, 1 + 4 + 4 + 16 + 12);
    CuAssertTrue(tc, OF_MSGTYPE_FULLLOG == pc.mtype);
    CuAssertTrue(tc, 16 == pc.fulllog.filelog_len);
    CuAssertTrue(tc, 12 == pc.fulllog.piecelog_len);
    CuAssertTrue(tc, 0 == strncmp(pc.fulllog.filelog,"ld4:path4:abcdee",16));
    CuAssertTrue(tc, 0 == strncmp(pc.fulllog.piecelog,"ld3:idxi1eee",12));
    of_msghandler_release(mh);
}

void Testof_pwp(
    CuTest * tc
)
{
    fake_pc_t pc;
    unsigned char data[100];
    unsigned char* ptr;
    void* mh;

    ptr = data;
    mh = of_msghandler_new(&pc, &((of_conn_recv_cb_t){
            .conn_keepalive = __conn_keepalive,
            .conn_fulllog = __conn_fulllog,
            .conn_pwp_dispatch = __conn_pwp_dispatch}));
    bitstream_write_ubyte(&ptr, OF_MSGTYPE_PWP);
    bitstream_write_uint32(&ptr, fe(20));
    bitstream_write_string(&ptr, "00000000000000000000", 20);
    of_msghandler_dispatch_from_buffer(mh, data, 1 + 4 + 20);
    CuAssertTrue(tc, OF_MSGTYPE_PWP == pc.mtype);
    CuAssertTrue(tc, 20 == pc.pwp.payload_len);
    CuAssertTrue(tc, 0 == strncmp(pc.pwp.payload,"00000000000000000000",20));
    of_msghandler_release(mh);
}

