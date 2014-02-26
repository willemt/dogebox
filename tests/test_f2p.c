
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "CuTest.h"

#include "bitfield.h"
#include "bitstream.h"
#include "dogebox.h"
#include "dogebox_connection.h"
#include "dogebox_msghandler.h"
#include "file2piece_mapper.h"

#include "bt.h"
#include "bt_piece_db.h"

void TestF2P_new_has_no_files(
    CuTest * tc
)
{
    f2p_t *f;

    f = f2p_new(NULL, 10);
    CuAssertTrue(tc, 0 == f2p_get_nfiles(f));
}

void TestF2P_added_adds_file(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *f;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, NULL == f2p_get_file_from_path(m, "test.txt"));
    f2p_file_added(m, "test.txt", 0, 5, 0);
    CuAssertTrue(tc, NULL != (f = f2p_get_file_from_path(m, "test.txt")));
    CuAssertTrue(tc, 0 == strcmp(f->path,"test.txt"));
}

