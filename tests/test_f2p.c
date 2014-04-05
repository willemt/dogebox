
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
    CuAssertTrue(tc, NULL != (f = f2p_file_added(m, "test.txt", 0, 5, 0)));
    CuAssertTrue(tc, f == (f2p_get_file_from_path(m, "test.txt")));
    CuAssertTrue(tc, 0 == strcmp(f->path,"test.txt"));
    CuAssertTrue(tc, 1 == f2p_get_nfiles(m));
}

void TestF2P_get_files_from_piece_idx(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *a;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, NULL != (a = f2p_file_added(m, "a.txt", 0, 5, 0)));
    CuAssertTrue(tc, a == f2p_get_files_from_piece_idx(m, 0));
}

void TestF2P_get_files_from_piece_idx_with_3_pieces(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *a, *b, *c;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, NULL != (a = f2p_file_added(m, "a.txt", 0, 5, 0)));
    CuAssertTrue(tc, NULL != (b = f2p_file_added(m, "b.txt", 0, 5, 0)));
    CuAssertTrue(tc, NULL != (c = f2p_file_added(m, "c.txt", 0, 5, 0)));
    CuAssertTrue(tc, a == f2p_get_files_from_piece_idx(m, 0));
    CuAssertTrue(tc, b == f2p_get_files_from_piece_idx(m, 1));
    CuAssertTrue(tc, c == f2p_get_files_from_piece_idx(m, 2));
}

#if 0
void T_estF2P_added_adds_file_to_specific_piece_idx(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *a;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, NULL != (a = f2p_file_added(m, "test.txt", 0, 5, 0, 1)));
    CuAssertTrue(tc, NULL == f2p_get_files_from_piece_idx(m, 1));
    CuAssertTrue(tc, a == f2p_get_files_from_piece_idx(m, 1));
    CuAssertTrue(tc, 1 == f2p_get_nfiles(m));
}
#endif

#if 0
void T_estF2P_added_adds_file_to_specific_piece_idx_remaps_when_conflicted(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *a, *b;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    a = f2p_file_added(m, "a.txt", 0, 5, 0, 1);
    b = f2p_file_added(m, "b.txt", 0, 5, 0, 1);
    CuAssertTrue(tc, b == f2p_get_files_from_piece_idx(m, 1));
    CuAssertTrue(tc, a == f2p_get_files_from_piece_idx(m, 0));
    CuAssertTrue(tc, 2 == f2p_get_nfiles(m));
}
#endif

void TestF2P_added_adds_two_files(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *f;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    f2p_file_added(m, "test1.txt", 0, 5, 0);
    f2p_file_added(m, "test2.txt", 0, 5, 0);
    CuAssertTrue(tc, NULL != (f = f2p_get_file_from_path(m, "test1.txt")));
    CuAssertTrue(tc, 0 == strcmp(f->path,"test1.txt"));
    CuAssertTrue(tc, NULL != (f = f2p_get_file_from_path(m, "test2.txt")));
    CuAssertTrue(tc, 0 == strcmp(f->path,"test2.txt"));
    CuAssertTrue(tc, 2 == f2p_get_nfiles(m));
}


void TestF2P_added_cant_add_file_twice(
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
    CuAssertTrue(tc, NULL == f2p_file_added(m, "test.txt", 0, 5, 0));
    CuAssertTrue(tc, 1 == f2p_get_nfiles(m));
}

void TestF2P_remove_removes_just_file(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *a, *b;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);

    CuAssertTrue(tc, NULL != (a = f2p_file_added(m, "a.txt", 0, 10, 0)));
    CuAssertTrue(tc, NULL != (b = f2p_file_added(m, "b.txt", 0, 10, 0)));
    CuAssertTrue(tc, a == f2p_get_files_from_piece_idx(m, 0));
    CuAssertTrue(tc, b == f2p_get_files_from_piece_idx(m, 1));

    CuAssertTrue(tc, a == f2p_file_removed(m, "a.txt"));
    CuAssertTrue(tc, a == f2p_get_file_from_path(m, "a.txt"));
    CuAssertTrue(tc, b == f2p_get_file_from_path(m, "b.txt"));
    CuAssertTrue(tc, a->is_deleted == 1);
    CuAssertTrue(tc, b->is_deleted != 1);

    CuAssertTrue(tc, 2 == f2p_get_nfiles(m));
    CuAssertTrue(tc, NULL == f2p_get_files_from_piece_idx(m, 0));
    CuAssertTrue(tc, NULL != f2p_get_files_from_piece_idx(m, 1));
    CuAssertTrue(tc, b == f2p_get_files_from_piece_idx(m, 1));
}

void TestF2P_pieces_required_for_filesize(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *f;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, 1 == f2p_pieces_required_for_filesize(m,0));
    CuAssertTrue(tc, 1 == f2p_pieces_required_for_filesize(m,1));
    CuAssertTrue(tc, 1 == f2p_pieces_required_for_filesize(m,10));
    CuAssertTrue(tc, 2 == f2p_pieces_required_for_filesize(m,11));
    CuAssertTrue(tc, 3 == f2p_pieces_required_for_filesize(m,21));
}

void TestF2P_added_adds_piece_range(
    CuTest * tc
)
{
    f2p_t *m;
    file_t *f;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, 0 == bt_piecedb_count(db));

    f2p_file_added(m, "test.txt", 0, 20, 0);
    CuAssertTrue(tc, 2 == bt_piecedb_count(db));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 0));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 1));
}

void TestF2P_remap_cant_remap_non_existant_file(CuTest * tc)
{
    f2p_t *m;
    file_t *f;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, NULL == f2p_file_remap(m, "test.txt", 1));
}

void TestF2P_remap_remaps(CuTest * tc)
{
    f2p_t *m;
    file_t *f;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, 0 == bt_piecedb_count(db));

    f2p_file_added(m, "test.txt", 0, 10, 0);
    CuAssertTrue(tc, 1 == bt_piecedb_count(db));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 0));
    CuAssertTrue(tc, NULL == bt_piecedb_get(db, 1));

    f2p_file_remap(m, "test.txt", 1);
    CuAssertTrue(tc, 1 == bt_piecedb_count(db));
    CuAssertTrue(tc, NULL == bt_piecedb_get(db, 0));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 1));
}

void TestF2P_remap_resolves_conflicts_by_taken_over_idx(CuTest * tc)
{
    f2p_t *m;
    file_t *a, *b;
    void *db;

    db = bt_piecedb_new();
    m = f2p_new(db, 10);
    CuAssertTrue(tc, 0 == bt_piecedb_count(db));

    a = f2p_file_added(m, "a.txt", 0, 10, 0);
    b = f2p_file_added(m, "b.txt", 0, 10, 0);
    CuAssertTrue(tc, 2 == bt_piecedb_count(db));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 0));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 1));
    CuAssertTrue(tc, a == f2p_get_files_from_piece_idx(m, 0));
    CuAssertTrue(tc, b == f2p_get_files_from_piece_idx(m, 1));

    f2p_file_remap(m, "b.txt", 0);
    CuAssertTrue(tc, 2 == bt_piecedb_count(db));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 0));
    CuAssertTrue(tc, NULL != bt_piecedb_get(db, 1));
    CuAssertTrue(tc, a == f2p_get_files_from_piece_idx(m, 1));
    CuAssertTrue(tc, b == f2p_get_files_from_piece_idx(m, 0));
}

