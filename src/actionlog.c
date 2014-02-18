
#define MAXPATH 100

enum {
    ACTIONLOG_MAP_PIECE,
    ACTIONLOG_UNMAP_PIECE,
    ACTIONLOG_CHANGE_PIECE,
    ACTIONLOG_REMOVE_FILE,
    ACTIONLOG_MOVE_FILE,

};

typedef struct 
{
    char path[MAX_PATH];
    int path_len;
    int offset;
    int piece_idx;
} action_map_piece_t;

typedef struct 
{
    char path[MAX_PATH];
    int path_len;
    int piece_idx;
} action_unmap_piece_t;

typedef struct 
{
    int piece_idx;
    char new_hash[20];
    int new_size;
} action_change_piece_t;

#if 0
typedef struct 
{
    void* file;
} action_file_remove_t;
#endif

typedef struct 
{
    char* new_path[MAX_PATH];
} action_file_move_t;



typedef struct 
{
    char type[50];
    int type_len;

    union {
        action_map_piece_t map_piece;
        action_unmap_piece_t unmap_piece;
        action_change_piece_t change_piece;
//        action_file_remove_t remove_file;
        action_file_move_t move_file;
    };
} action_t;

static void __process_action(f2p_t* f2p, bencode_t* dict)
{
    // TODO: switch away from path
    char path[1000];
    int pathlen = 0;
    int fsize = 0;
    int piece_idx_start = 0;
    int pieces = 0;
    unsigned int mtime = 0;

    action_t a;

    while (bencode_dict_has_next(&dict))
    {
        bencode_t benk;
        const char *key;
        int klen;

        bencode_dict_get_next(&dict, &benk, &key, &klen);

        if (!strncmp(key, "log_id", klen))
        {
            bencode_int_value(&benk, &logid);
        }
        else if (!strncmp(key, "action_type", klen))
        {
            bencode_string_value(&benk, &a.type, &a.type_len);

        }
        else
        {
            if (0 == strncmp(a.type,"map_piece",50))
            {
                if (!strncmp(key, "path", klen))
                    bencode_string_value(&benk, &a.map_piece.path, &a.map_piece.pathlen);
                else if (!strncmp(key, "offset", klen))
                    bencode_int_value(&benk, &a.map_piece.offset);
                else if (!strncmp(key, "piece_idx", klen))
                    bencode_int_value(&benk, &a.map_piece.piece_idx);
            }
            else if (0 == strncmp(a.type,"unmap_piece",50))
            {
                if (!strncmp(key, "path", klen))
                    bencode_string_value(&benk, &a.unmap_piece.path, &a.unmap_piece.pathlen);
                else if (!strncmp(key, "piece_idx", klen))
                    bencode_int_value(&benk, &a.map_piece.piece_idx);
                else if (!strncmp(key, "new_size", klen))
                    bencode_int_value(&benk, &a.unmap_piece.new_size);
            }
            else if (0 == strncmp(a.type,"change_piece",50))
            {
                if (!strncmp(key, "piece_idx", klen))
                    bencode_string_value(&benk, &a.type, &a.type_len);
                else if (!strncmp(key, "new_hash", klen))
                    bencode_string_value(&benk, &a.type, &a.type_len);
            }
            else if (0 == strncmp(a.type,"move_file",50))
            {

                if (!strncmp(key, "path", klen))
                    bencode_int_value(&benk, &logid);
            }
            else if (0 == strncmp(a.type,"remove_file",50))
            {
                char buf[100];
                int buf_len;
                bencode_string_value(&benk, &buf, &buf_len);
                // TODO: dangerous without strlen
                f2p_removed(f2p,buf);
            }
        }
    }
}

// TODO: custom message handler needs to pass sys_t instead of pc
void of_conn_actionlog(void* pc, const unsigned char* buf, unsigned int len)
{
    bencode_t list;

    printf("Received filelog: '%.*s'\n", len, buf);

    bencode_init(&list, buf, len);
    if (!bencode_is_list(&list))
    {
        printf("bad file log, expected list\n");
        return;
    }

    while (bencode_list_has_next(&list))
    {
        bencode_t dict;

        bencode_list_get_next(&list, &dict);

        __process_action(f2p, &dict);
    }
}
