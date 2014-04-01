
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

#include "file2piece_mapper.h"
#include "bt.h"
#include "bt_piece_db.h"


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
            "5:mtimei%de"
            "e"
            "e",
            strlen("testing/123.txt"), "testing/123.txt",
            10, /* size */
            1, /* piece_idx */
            1 /* mtime */);

    CuAssertTrue(tc, 1 == of_conn_filelog(c, msg, ptr - msg));

}

/* FL01 */
void Testof_receives_filelog_adds_file_if_we_dont_have_it(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    of_conn_t* c;

    char* file_name = "testing/123.txt";

    c = of_conn_new(
            &((of_conn_cb_t){
            .conn_pwp_dispatch = __conn_pwp_dispatch
            }), NULL);

    void* db = bt_piecedb_new();
    void* pm = f2p_new(db, 1 << 21);
    of_conn_set_piece_mapper(c, pm);

    CuAssertTrue(tc, !f2p_get_file_from_path(pm, file_name));

    ptr += sprintf(ptr,
            "l"
            "d"
            "4:path%d:%s"
            "4:sizei%de"
            "10:is_deleted1:y"
            "9:piece_idxi%de"
            "5:mtimei%de"
            "e"
            "e",
            strlen(file_name), file_name,
            10, /* size */
            1, /* piece_idx */
            1 /* mtime */);

    CuAssertTrue(tc, 1 == of_conn_filelog(c, msg, ptr - msg));
    CuAssertTrue(tc, 1 == f2p_get_nfiles(pm));
    CuAssertTrue(tc, NULL != f2p_get_file_from_path(pm, file_name));
}

/* FL02 */
void Testof_receives_filelog_adds_piece_range_if_we_dont_have_it(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    of_conn_t* c;

    char* file_name = "testing/a.txt";

    c = of_conn_new(
            &((of_conn_cb_t){
            .conn_pwp_dispatch = __conn_pwp_dispatch
            }), NULL);

    void* db = bt_piecedb_new();
    void* pm = f2p_new(db, 1 << 21);
    of_conn_set_piece_mapper(c, pm);

    CuAssertTrue(tc, 0 == bt_piecedb_count(db));
    CuAssertTrue(tc, !f2p_get_file_from_path(pm, file_name));

    ptr += sprintf(ptr,
            "l"
            "d"
            "4:path%d:%s"
            "4:sizei%de"
            "10:is_deleted1:%s"
            "9:piece_idxi%de"
            "5:mtimei%de"
            "e"
            "e",
            strlen(file_name), file_name,
            10, /* size */
            "y", /* is_deleted */
            1, /* piece_idx */
            1 /* mtime */);
    CuAssertTrue(tc, 1 == of_conn_filelog(c, msg, ptr - msg));
    printf("files: %d\n", f2p_get_nfiles(pm));
    CuAssertTrue(tc, 1 == f2p_get_nfiles(pm));
    CuAssertTrue(tc, NULL != f2p_get_file_from_path(pm, file_name));
    CuAssertTrue(tc, 1 == bt_piecedb_count(db));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 0));
    CuAssertTrue(tc, NULL == bt_piecedb_get(db, 1));

    ptr = msg;
    file_name = "testing/b.txt";
    ptr += sprintf(ptr,
            "l"
            "d"
            "4:path%d:%s"
            "4:sizei%de"
            "10:is_deleted1:%s"
            "9:piece_idxi%de"
            "5:mtimei%de"
            "e"
            "e",
            strlen(file_name), file_name,
            10, /* size */
            "y", /* is_deleted */
            2, /* piece_idx */
            1 /* mtime */);
    CuAssertTrue(tc, 1 == of_conn_filelog(c, msg, ptr - msg));
    CuAssertTrue(tc, 2 == f2p_get_nfiles(pm));
    CuAssertTrue(tc, NULL != f2p_get_file_from_path(pm, file_name));
    printf("pieces: %d\n", bt_piecedb_count(db)); 
    CuAssertTrue(tc, 3 == bt_piecedb_count(db));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 0));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 1));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 2));
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
            "4:hash%d:%s"
            "5:mtimei%de"
            "e"
            "e",
            10, /* idx */
            10, /* size */
            strlen("testing/123.txt"), "testing/123.txt", /* hash */
            1 /* mtime */
            );

    CuAssertTrue(tc, 0 == of_conn_piecelog(c, msg, ptr - msg));
}

