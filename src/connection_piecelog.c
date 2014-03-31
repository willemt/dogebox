
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

#if 0
typedef struct {

} file_t;
#endif

int connection_pl_int(bencode_t *s,
        const char *dict_key,
        const long int val)
{
    if (!strcmp(dict_key, "idx"))
    {

    }
    else if (!strcmp(dict_key, "size"))
    {

    }
    else if (!strcmp(dict_key, "mtime"))
    {

    }
    else
    {
        assert(0);
    }


    return 1;
}

int connection_pl_str(bencode_t *s,
        const char *dict_key,
        unsigned int v_total_len __attribute__((__unused__)),
        const unsigned char* val,
        unsigned int v_len) 
{
    if (!strcmp(dict_key, "hash"))
    {

    }
    else
    {
        assert(0);
    }

    return 1;
}

