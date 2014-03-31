#ifndef DOGEBOX_CONNECTION_PRIVATE_H
#define DOGEBOX_CONNECTION_PRIVATE_H

typedef struct {
    pwp_conn_private_t pwp_conn;
    void* udata;

    /* piecemapper */
    void* pm;

    /* piece db */
    void* db;

    /* piece log reader */
    bencode_t* pl_reader;

    /* file log reader */
    bencode_t* fl_reader;

    /* current file we are reading from wire */
    file_t file;

} conn_private_t;

int connection_pl_int(bencode_t *s,
        const char *dict_key,
        const long int val);

int connection_pl_str(bencode_t *s,
        const char *dict_key,
        unsigned int v_total_len __attribute__((__unused__)),
        const unsigned char* val,
        unsigned int v_len);

int connection_fl_int(bencode_t *s,
        const char *dict_key,
        const long int val);

int connection_fl_str(bencode_t *s,
        const char *dict_key,
        unsigned int v_total_len __attribute__((__unused__)),
        const unsigned char* val,
        unsigned int v_len);

//int connection_fl_list_next(bencode_t *s);
int connection_fl_dict_leave(bencode_t *s, const char *dict_key);

#endif /* DOGEBOX_CONNECTION_PRIVATE_H */
