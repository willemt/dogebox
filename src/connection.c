
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* for uint32_t */
#include <stdint.h>

#include "bitfield.h"
#include "dogebox.h"
#include "dogebox_connection.h"
#include "dogebox_msghandler.h"

/* for sys_t */
#include "linked_list_queue.h"

/* for iterating through f2p hashmap */
#include "linked_list_hashmap.h"

/* for f2p_t */
#include "file2piece_mapper.h"

/* dogebox local needs dm_stats_t */
#include "bt.h"

/* for piece database */
#include "bt_piece_db.h"

/* for sys_t */
//#include "dogebox_local.h"

/* for filelog reading */
#include "bencode.h"

/* for piggy backing on pwp_conn_private_t */
#include "sparse_counter.h"
#include "pwp_connection.h"
#include "pwp_connection_private.h"

#include "dogebox_connection_private.h"

static bencode_callbacks_t __fl_cb = {
    .hit_int = connection_fl_int,
    .hit_str = connection_fl_str,
    .dict_enter = NULL,
    .dict_leave = connection_fl_dict_leave,
    .list_enter = NULL,
    .list_leave = NULL,
    .list_next = NULL
};

static bencode_callbacks_t __pl_cb = {
    .hit_int = connection_pl_int,
    .hit_str = connection_pl_str,
    .dict_enter = NULL,
    .dict_leave = NULL,
    .list_enter = NULL,
    .list_leave = NULL,
    .list_next = NULL
};

of_conn_t* of_conn_new(of_conn_cb_t* cb, void* udata)
{
    conn_private_t *me;

    me = calloc(1, sizeof(conn_private_t));
    me->udata = udata;
    me->fl_reader = bencode_new(10, &__fl_cb, me);
    me->pl_reader = bencode_new(10, &__pl_cb, me);
    return (of_conn_t*)me;
}

void of_conn_set_piece_mapper(of_conn_t* me_, void* pm)
{
    conn_private_t* me = (void*)me_;
    me->pm = pm;
}

void of_conn_set_piece_db(of_conn_t* me_, void* db)
{
    conn_private_t* me = (void*)me_;
    me->db = db;
}

int of_conn_filelog(void* pc, const unsigned char* buf, unsigned int len)
{
    conn_private_t* me = pc;      
    bencode_t ben;

    printf("Received filelog: '%.*s'\n", len, buf);
    if (1 != bencode_dispatch_from_buffer(me->fl_reader, buf, len))
    {
        printf("ERROR reading: %.*s\n", len, buf);
        return 0;
    }

    return 1;
}

int of_conn_piecelog(void* pc, const unsigned char* buf, unsigned int len)
{
    conn_private_t* me = pc;      
    bencode_t ben;

    printf("Received piecelog: '%.*s'\n", len, buf);

    /* piece database */
    //void* db = me->db;

    if (1 != bencode_dispatch_from_buffer(me->pl_reader, buf, len))
    {
        printf("ERROR reading: %.*s\n", len, buf);
        return 0;
    }

    return 1;
}
