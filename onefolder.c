
/**
 * Copyright (c) 2011, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. 
 *
 * @file
 * @author  Willem Thiart himself@willemthiart.com
 */

/*
New_file file, size hint
Enlarge_file file size
reduce_size
Delete_file
Filepos_getpiece file, pos
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/time.h>
#include <uv.h>

/* for INT_MAX */
#include <limits.h>

#include "bt.h"
#include "bt_piece_db.h"
#include "bt_diskcache.h"
#include "bt_filedumper.h"
#include "bt_string.h"
#include "bt_sha1.h"
#include "bt_selector_random.h"
#include "config.h"
#include "networkfuncs.h"
#include "linked_list_queue.h"
#include "filewatcher.h"

#include "docopt.c"

#define PROGRAM_NAME "bt"

typedef struct {
    /* bitorrent client */
    void* bc;

    /* piece db*/
    void* db;

    /* file dumper */
    void* fd;

    /* disk cache */
    void* dc;

    /* configuration */
    void* cfg;

    /* tracker client */
    void* tc;

    bt_dm_stats_t stat;

    uv_mutex_t mutex;

    /* filewatcher */
    void* fw;
} sys_t;

uv_loop_t *loop;

static void __log(void *udata, void *src, const char *buf, ...)
{
    char stamp[32];
//    int fd = (unsigned long) udata;
    struct timeval tv;

    //printf("%s\n", buf);
    gettimeofday(&tv, NULL);
    sprintf(stamp, "%d,%0.2f,", (int) tv.tv_sec, (float) tv.tv_usec / 100000);
//    write(fd, stamp, strlen(stamp));
//    write(fd, buf, strlen(buf));
}

static void* on_call_exclusively(void* me, void* cb_ctx, void **lock, void* udata,
        void* (*cb)(void* me, void* udata))
{
    void* result;

    if (NULL == *lock)
    {
        *lock = malloc(sizeof(uv_mutex_t));
        uv_mutex_init(*lock);
    }

    uv_mutex_lock(*lock);
    result = cb(me,udata);
    uv_mutex_unlock(*lock);
    return result;
}

static int __dispatch_from_buffer(
        void *callee,
        void *peer_nethandle,
        const unsigned char* buf,
        unsigned int len)
{
    sys_t* me = callee;

    uv_mutex_lock(&me->mutex);
    bt_dm_dispatch_from_buffer(me->bc,peer_nethandle,buf,len);
    uv_mutex_unlock(&me->mutex);
    return 1;
}

static int __on_peer_connect(
        void *callee,
        void* peer_nethandle,
        char *ip,
        const int port)
{
    sys_t* me = callee;

    uv_mutex_lock(&me->mutex);
    bt_dm_peer_connect(me->bc,peer_nethandle,ip,port);
    uv_mutex_unlock(&me->mutex);

    return 1;
}

static void __on_peer_connect_fail(
    void *callee,
    void* peer_nethandle)
{
    sys_t* me = callee;

    uv_mutex_lock(&me->mutex);
    bt_dm_peer_connect_fail(me->bc,peer_nethandle);
    uv_mutex_unlock(&me->mutex);
}

static void __periodic(uv_timer_t* handle, int status)
{
    sys_t* me = handle->data;
    int i;

    if (me->bc)
    {
        uv_mutex_lock(&me->mutex);
        bt_dm_periodic(me->bc, &me->stat);
        uv_mutex_unlock(&me->mutex);
    }

    //__log_process_info();
}

int file_added(
    void* udata,
    char* name,
    int is_dir,
    unsigned int size,
    unsigned long mtime)
{
    printf("added: %s %dB %d\n", name, size, is_dir);
    return 0;
}

int file_removed(void* udata, char* name)
{
    printf("removed: %s\n", name);
    return 0;
}

int file_changed(void* udata, char* name, int new_size, unsigned long mtime)
{
    printf("changed: %s %d\n", name, new_size);
    return 0;
}

int file_moved(void* udata, char* name, char* new_name, unsigned long mtime)
{
    printf("moved: %s %s\n", name, new_name);
    return 0;
}

int main(int argc, char **argv)
{
    DocoptArgs args = docopt(argc, argv, 1, "0.1");
    sys_t me;

    me.bc = bt_dm_new();
    me.cfg = bt_dm_get_config(me.bc);
    memset(&me.stat, 0, sizeof(bt_dm_stats_t));

#if 0
    status = config_read(cfg, "yabtc", "config");
    setlocale(LC_ALL, " ");
    atexit (close_stdin);
    bt_dm_set_logging(bc,
            open("dump_log", O_CREAT | O_TRUNC | O_RDWR, 0666), __log);
#endif

    config_set(me.cfg, "my_peerid", bt_generate_peer_id());
    assert(config_get(me.cfg, "my_peerid"));

    /* database for dumping pieces to disk */
    me.fd = bt_filedumper_new();

    /* Disk Cache */
    me.dc = bt_diskcache_new();
    /* point diskcache to filedumper */
    bt_diskcache_set_disk_blockrw(me.dc,
            bt_filedumper_get_blockrw(me.fd), me.fd);

    /* Piece DB */
    me.db = bt_piecedb_new();
    bt_piecedb_set_diskstorage(me.db,
            bt_diskcache_get_blockrw(me.dc), me.dc);
    bt_dm_set_piece_db(me.bc,
            &((bt_piecedb_i) {
            .get_piece = bt_piecedb_get
            }), me.db);

    /* Selector */
    bt_dm_set_piece_selector(me.bc, &((bt_pieceselector_i) {
                .new = bt_random_selector_new,
                .peer_giveback_piece = bt_random_selector_giveback_piece,
                .have_piece = bt_random_selector_have_piece,
                .remove_peer = bt_random_selector_remove_peer,
                .add_peer = bt_random_selector_add_peer,
                .peer_have_piece = bt_random_selector_peer_have_piece,
                .get_npeers = bt_random_selector_get_npeers,
                .get_npieces = bt_random_selector_get_npieces,
                .poll_piece = bt_random_selector_poll_best_piece }), NULL);
    bt_dm_check_pieces(me.bc);

    /* set network functions */
    bt_dm_set_cbs(me.bc, &((bt_dm_cbs_t) {
            .peer_connect = peer_connect,
            .peer_send = peer_send,
            .peer_disconnect = peer_disconnect, 
            .call_exclusively = on_call_exclusively,
            .log = __log
            }), NULL);

    if (argc == optind)
    {
        printf("%s", args.help_message);
        exit(EXIT_FAILURE);
    }

    /* start uv */
    loop = uv_default_loop();
    uv_mutex_init(&me.mutex);

    if (args.folder)
    {
        me.fw = filewatcher_new(args.folder, loop,
            &((filewatcher_cbs_t){
                .file_added = file_added, 
                .file_removed = file_removed, 
                .file_changed = file_changed, 
                .file_moved = file_moved
                }), &me);

    }

    /* create periodic timer */
    uv_timer_t *periodic_req;
    periodic_req = malloc(sizeof(uv_timer_t));
    periodic_req->data = &me;
    uv_timer_init(loop, periodic_req);
    uv_timer_start(periodic_req, __periodic, 0, 1000);

    uv_run(loop, UV_RUN_DEFAULT);

    bt_dm_release(me.bc);
    return 1;
}

