
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

/* for filelog reading */
#include "bencode.h"

/* for piggy backing on pwp_conn_private_t */
#include "sparse_counter.h"
#include "pwp_connection.h"
#include "pwp_connection_private.h"

#include "dogebox_connection_private.h"

int connection_fl_int(bencode_t *s,
        const char *dict_key,
        const long int val)
{
    conn_private_t *me = (void*)s->udata;

    if (!strcmp(dict_key, "size"))
    {
        me->file.size = val;
    }
    else if (!strcmp(dict_key, "piece_idx"))
    {
        me->file.piece_start = val;
    }
#if 0
    else if (!strcmp(dict_key, "pieces"))
    {
        me->file.npieces = val;
    }
#endif
    else if (!strcmp(dict_key, "mtime"))
    {
        me->file.mtime = val;
    }
    else
    {
        printf("can't recognise %s for int\n", dict_key);
        assert(0);
    }

    return 1;
}

int connection_fl_str(bencode_t *s,
        const char *dict_key,
        unsigned int v_total_len __attribute__((__unused__)),
        const unsigned char* val,
        unsigned int v_len) 
{
    conn_private_t *me = (void*)s->udata;

    if (!strcmp(dict_key, "path"))
    {
        if (me->file.path_len < v_len)
        {
            me->file.path_len = v_len+1;
            me->file.path = realloc(me->file.path, v_len+1);
        }
        strncpy(me->file.path,val,v_len);
        me->file.path[v_len] = 0;
    }
    else if (!strcmp(dict_key, "is_deleted"))
    {
        assert(v_len == 1);
        if (*val == 'y') 
        {
            me->file.is_deleted = 1;
        }
    }
    else
    {
        printf("can't recognise %s for string\n", dict_key);
        assert(0);
    }

    return 1;
}

int connection_fl_dict_leave(bencode_t *s, const char *dict_key)
{
    conn_private_t *me = (void*)s->udata;

    if (!me->pm)
        return 1;

    /* b: new file (from peer) */
    file_t* b = &me->file;

    /* a: old file (currently in our register) */
    file_t* a = f2p_get_file_from_path(me->pm, b->path);

    if (!a)
    {
        a = f2p_file_added(me->pm, b->path, 0, b->size, b->mtime, b->piece_start);
    }
    else if (a->mtime < b->mtime)
    {
        if (1 == b->is_deleted)
        {
            f2p_file_removed(me->pm, a->path);
        }
        else if (a->size != b->size)
        {
            f2p_file_changed(me->pm, b->path, b->size, b->mtime);
        }
        else
        {
//            f2p_file_remap(me->pm, b->path, b->piece_start, b->npieces);
        }

        if (a->piece_start != b->piece_start)
        {
            f2p_file_remap(me->pm, a->path, b->piece_start);//, b->npieces);
        }
    }

    return 1;
}

