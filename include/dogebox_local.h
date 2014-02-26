
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


