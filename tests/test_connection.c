
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "CuTest.h"

#include "bitfield.h"
#include "dogebox.h"
#include "dogebox_connection.h"
#include "dogebox_handshaker.h"
#include "bitstream.h"

static int __conn_pwp_dispatch(
        void *mh,
        const unsigned char* buf,
        unsigned int len)
{
    return 1;
}

void Testof_receives_filelog(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    of_conn_t* c;

    c = of_conn_new(
            &((of_conn_cb_t){
            .conn_pwp_dispatch = __conn_pwp_dispatch
            }), NULL);

    ptr += sprintf(ptr,
            "l"
            "d"
            "4:path%d:%s"
            "4:sizei%de"
            "10:is_deleted1:y"
            "9:piece_idxi%de"
            "6:piecesi%de"
            "5:mtimei%de"
            "e"
            "e",
            strlen("testing/123.txt"), "testing/123.txt",
            10, /* size */
            1, 1, 1);

    CuAssertTrue(tc, 1 == of_conn_filelog(c, msg, ptr - msg));
}

void Testof_receives_piecelog(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    of_conn_t* c;

    c = of_conn_new(
            &((of_conn_cb_t){
            .conn_pwp_dispatch = __conn_pwp_dispatch
            }), NULL);

    ptr += sprintf(ptr,
            "l"
            "d"
            "3:idxi%de"
            "4:sizei%de"
            "4:hash%d:%s"
            "5:mtimei%de"
            "e"
            "e",
            10, /* idx */
            10, /* size */
            strlen("testing/123.txt"), "testing/123.txt", /* idx */
            1 /* mtime */
            );

    CuAssertTrue(tc, 1 == of_conn_piecelog(c, msg, ptr - msg));
}

void Testof_piecelog_needs_to_have_hash_of_20len(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    of_conn_t* c;

    c = of_conn_new(
            &((of_conn_cb_t){
            .conn_pwp_dispatch = __conn_pwp_dispatch
            }), NULL);

    ptr += sprintf(ptr,
            "l"
            "d"
            "3:idxi%de"
            "4:sizei%de"
            "4:hash%se"
            "5:mtimei%de"
            "e"
            "e",
            10, /* idx */
            10, /* size */
            strlen("testing/123.txt"), "testing/123.txt", /* idx */
            1 /* mtime */
            );

    CuAssertTrue(tc, 0 == of_conn_piecelog(c, msg, ptr - msg));
}

