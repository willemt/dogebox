
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "CuTest.h"

#include "bt.h"
#include "bt_piece_db.h"
#include "bt_diskcache.h"
#include "bt_string.h"
#include "bt_selector_random.h"

#include "networkfuncs.h"

/* for iterating through f2p hashmap */
#include "linked_list_hashmap.h"

/* for f2p_t */
#include "file2piece_mapper.h"

/* for msg_fulllog_t */
#include "dogebox.h"

#include "dogebox_handshaker.h"

#include "dogebox_connection.h"

/* for of_msghandler_new() */
#include "dogebox_msghandler.h"

/* for piggy backing on PWP_ msg types */
#include "bitfield.h"
#include "pwp_connection.h"

/* for pwp_msghandler_item_t */
#include "pwp_msghandler.h"

/* for filewatcher */
#include "fff.h"

/* for command line options */
#include "docopt.c"

/* for sys_t */
#include "dogebox_local.h"

void Testof_receives_filelog_adds_file_if_we_dont_have_it(
    CuTest * tc
)
{
    sys_t me;

    memset(&me,0,sizeof(sys_t));
    me.bc = bt_dm_new();
    me.cfg = bt_dm_get_config(me.bc);

    config_set_va(me.cfg, "npieces", "%u", 1 << 31),
    config_set_va(me.cfg, "piece_length", "%d", 1 << 21);
    config_set(me.cfg, "my_peerid", bt_generate_peer_id());

    /* 2mb pieces */
    me.pm = f2p_new(me.db, 1 << 21);

    me.dc = bt_diskcache_new();
    bt_diskcache_set_disk_blockrw(me.dc,
            &((bt_blockrw_i){
                .write_block = f2p_write_block,
                .read_block = f2p_read_block,
                .flush_block = f2p_flush_block
                }), me.pm);

    me.db = bt_piecedb_new();
    bt_piecedb_set_diskstorage(me.db,
            bt_diskcache_get_blockrw(me.dc), me.dc);
    bt_dm_set_piece_db(me.bc,
            &((bt_piecedb_i) {
            .get_piece = bt_piecedb_get
            }), me.db);

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

    bt_dm_set_cbs(me.bc, &((bt_dm_cbs_t) {
            .peer_connect = peer_connect,
            .peer_send = peer_send,
            .peer_disconnect = peer_disconnect, 
            .call_exclusively = on_call_exclusively,
            .log = __log,
            .handshaker_new = of_handshaker_new,
            .handshaker_release = of_handshaker_release,
            .handshaker_dispatch_from_buffer = of_handshaker_dispatch_from_buffer,
            .send_handshake = of_send_handshake,
            .handshake_success = __handshake_success,
            .msghandler_new = __new_msghandler,
            }), &me);

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
        char* port = strstr(args.connect, ":");

        if (port)
        {
            printf("connecting to: %s\n", args.connect);
            __on_tc_add_peer(&me, "", 0,
                    args.connect, port - args.connect, atoi(port+1));
        }
    }

    config_set_va(me.cfg, "pwp_listen_port", "%d", listen_port);
    printf("Listening on port: %d\n", listen_port);

    config_print(me.cfg);

    /* create periodic timer */
    uv_timer_t *periodic_req;
    periodic_req = malloc(sizeof(uv_timer_t));
    periodic_req->data = &me;
    uv_timer_init(loop, periodic_req);
    uv_timer_start(periodic_req, __periodic, 0, 1000);

    uv_run(loop, UV_RUN_DEFAULT);

    bt_dm_release(me.bc);
}

