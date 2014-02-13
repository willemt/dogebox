
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

/* for f2p_t */
#include "file2piece_mapper.h"

#include "onefolder_handshaker.h"

/* for of_msghandler_new() */
#include "onefolder_msghandler.h"

/* for pwp_msghandler_item_t */
#include "pwp_msghandler.h"

/* for filewatcher */
#include "fff.h"

#include "docopt.c"

#define PROGRAM_NAME "bt"

enum {
    OF_MSGTYPE_FILELOG = 9,
    OF_MSGTYPE_PIECELOG = 10,
};

typedef struct {
    /* bitorrent client */
    void* bc;

    /* piece db*/
    void* db;

    /* file dumper */
    void* fd;

    /* disk cache */
    void* dc;

    /* piece mapper */
    f2p_t* pm;

    /* configuration */
    void* cfg;

    /* tracker client */
    void* tc;

    /* of message handler */
    void* mh;

    bt_dm_stats_t stat;

    uv_mutex_t mutex;

    /* filewatcher */
    filewatcher_t* fw;
} sys_t;

uv_loop_t *loop;

static void __log(void *udata, void *src, const char *buf, ...)
{
#if 0
    char stamp[32];
    int fd = (unsigned long) udata;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sprintf(stamp, "%d,%0.2f,", (int) tv.tv_sec, (float) tv.tv_usec / 100000);
    write(fd, stamp, strlen(stamp));
    write(fd, buf, strlen(buf));
#endif

    printf("LOG: %s\n", buf);
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

    printf("got some data: %d\n", len);

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

    printf("peer wants to connect %s:%d\n", ip, port);

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
        if (me->fw)
            fff_periodic(me->fw,1000);
        bt_dm_periodic(me->bc, &me->stat);
        uv_mutex_unlock(&me->mutex);
    }

    //__log_process_info();
}

int file_added(
    void* callee,
    char* name,
    int is_dir,
    unsigned int size,
    unsigned long mtime)
{
    sys_t* me = callee;

    printf("added: %s %dB %d\n", name, size, is_dir);
    f2p_file_added(me->pm, name, is_dir, size, mtime);
    return 0;
}

int file_removed(void* callee, char* name)
{
    sys_t* me = callee;
    printf("removed: %s\n", name);
    f2p_file_removed(me->pm, name);
    return 0;
}

int file_changed(void* callee, char* name, int new_size, unsigned long mtime)
{
    sys_t* me = callee;
    printf("changed: %s %d\n", name, new_size);
    f2p_file_changed(me->pm, name, new_size, mtime);
    return 0;
}

int file_moved(void* callee, char* name, char* new_name, unsigned long mtime)
{
    sys_t* me = callee;
    printf("moved: %s %s\n", name, new_name);
    f2p_file_moved(me->pm, name, new_name, mtime);
    return 0;
}

static void* __new_msghandler(
        void *callee,
        void *pc)
{
    sys_t* me = callee;

    return of_msghandler_new(pc, callee);
}

static int __pwp_dispatch_from_buffer(
        void *callee,
        const unsigned char* buf,
        unsigned int len)
{
    sys_t* me = callee;

    uv_mutex_lock(&me->mutex);
//    bt_dm_dispatch_from_buffer(me->bc,peer_nethandle,buf,len);
    uv_mutex_unlock(&me->mutex);

    return 1;
}

static void __on_tc_add_peer(void* callee,
        char* peer_id,
        unsigned int peer_id_len,
        char* ip,
        unsigned int ip_len,
        unsigned int port)
{
    sys_t* me = callee;
    void* peer;
    void* netdata;
    void* peer_nethandle;
    char ip_string[32];

    peer_nethandle = NULL;
    sprintf(ip_string,"%.*s", ip_len, ip);

#if 1 /* debug */
    printf("adding peer: %s %d\n", ip_string, port);
#endif

    uv_mutex_lock(&me->mutex);
    if (0 == peer_connect(me, &netdata, &peer_nethandle, ip_string, port,
                __dispatch_from_buffer,
                __on_peer_connect,
                __on_peer_connect_fail))
    {

    }
    peer = bt_dm_add_peer(me->bc, peer_id, peer_id_len, ip, ip_len, port, peer_nethandle);
    uv_mutex_unlock(&me->mutex);
}

int main(int argc, char **argv)
{
    DocoptArgs args = docopt(argc, argv, 1, "0.1");
    sys_t me;

    memset(&me,0,sizeof(sys_t));
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

    me.dc = bt_diskcache_new();
    me.fd = bt_filedumper_new();
    bt_diskcache_set_disk_blockrw(me.dc,
            bt_filedumper_get_blockrw(me.fd), me.fd);

    me.db = bt_piecedb_new();
    bt_piecedb_set_diskstorage(me.db,
            bt_diskcache_get_blockrw(me.dc), me.dc);
    bt_dm_set_piece_db(me.bc,
            &((bt_piecedb_i) {
            .get_piece = bt_piecedb_get
            }), me.db);

    /* 2mb pieces */
    me.pm = f2p_new(me.db, 1 << 21);

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
            .log = __log,
            .handshaker_new = of_handshaker_new,
            .handshaker_release = of_handshaker_release,
            .handshaker_dispatch_from_buffer = of_handshaker_dispatch_from_buffer,
            .handshaker_send_handshake = of_handshaker_send_handshake,
            .msghandler_new = __new_msghandler
            //.msghandler_dispatch_from_buffer = __pwp_dispatch_from_buffer,
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
        printf("watching folder: %s\n", args.folder);
        me.fw = fff_new(args.folder, loop,
            &((filewatcher_cbs_t){
                .file_added = file_added, 
                .file_removed = file_removed, 
                .file_changed = file_changed, 
                .file_moved = file_moved
                }), &me);

    }

    /* open listening port */
    void* netdata;
    int listen_port = args.port ? atoi(args.port) : 0;
    if (0 == (listen_port = peer_listen(&me, &netdata, listen_port,
                __dispatch_from_buffer,
                __on_peer_connect,
                __on_peer_connect_fail)))
    {
        printf("ERROR: can't create listening socket");
        exit(0);
    }

    if (args.connect)
    {
        char* port;

        if ((port = strstr(args.connect, ":")))
        {
            printf("connecting to: %s\n", args.connect);
            __on_tc_add_peer(&me, "", 0,
                    args.connect, port - args.connect, atoi(port+1));
        }
    }

    config_set_va(me.cfg, "pwp_listen_port", "%d", listen_port);
    printf("Listening on port: %d\n", listen_port);

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

