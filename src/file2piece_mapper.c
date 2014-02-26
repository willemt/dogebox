
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

#include "file2piece_mapper.h"
#include "linked_list_hashmap.h"

#include "bt.h"
#include "bt_piece_db.h"

typedef struct {
    hashmap_t *files;
    void* piecedb;
    unsigned int piece_size;
} f2p_private_t;

/**
 * djb2 by Dan Bernstein. */
static unsigned long __file_hash(const void *obj)
{
    const file_t* f = obj;
    const char* str;
    unsigned long hash = 5381;
    int c;
    
    for (str = f->path; c = *str++;)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static long __file_compare(const void *obj, const void *other)
{
    const file_t* f1 = obj;
    const file_t* f2 = other;
    return strcmp(f1->path, f2->path);
}

f2p_t* f2p_new(void* piecedb, unsigned int piece_size)
{
    f2p_private_t* me;
    me = calloc(1,sizeof(f2p_private_t));
    me->files = hashmap_new(__file_hash, __file_compare, 11);
    me->piecedb = piecedb;
    me->piece_size = piece_size;
    return (f2p_t*)me;
}

static unsigned int __pieces_required(
        unsigned int size, unsigned int piece_size)
{
    if (size < piece_size)
        return 1;
    else
        return piece_size / size + (0 != (size % piece_size) ? 1 : 0);
}

void* f2p_file_added(
    f2p_t* me_,
    char* name,
    int is_dir,
    unsigned int size,
    unsigned long mtime)
{
    f2p_private_t* me = (void*)me_;
    int idx, npieces, i;

    printf("added: %s %dB %d\n", name, size, is_dir);

    file_t* f;
        
    f = f2p_get_file_from_path(me_, name);

    assert(!f);

    f = calloc(1, sizeof(file_t));
    f->path = strdup(name);
    f->is_dir = is_dir;
    f->size = size;
    f->mtime = mtime;
    hashmap_put(me->files, f->path, f);

    npieces = __pieces_required(size, me->piece_size);
    idx = bt_piecedb_add(me->piecedb, npieces);
    for (i=0; i<npieces; i++)
    {
        void* p = bt_piecedb_get(me->piecedb, idx + i);
        int piece_size = me->piece_size;
        char hash[20];

        /* last piece has special size */
        if (i == npieces - 1)
            piece_size = size < me->piece_size ? size : size % me->piece_size; 
        bt_piece_set_size(p, piece_size);
        bt_piece_calculate_hash(p, hash);
        bt_piece_set_hash(p, hash);

        // TODO: send notification
    }

    return NULL;
}

void* f2p_file_removed(
    f2p_t* me_,
    char* name)
{
    printf("removed: %s\n", name);
    return NULL;
}

void* f2p_file_changed(
    f2p_t* me_, char* name, int new_size, unsigned long mtime)
{
    printf("changed: %s %d\n", name, new_size);
    return NULL;
}

void* f2p_file_moved(
    f2p_t* me_, char* name, char* new_name, unsigned long mtime)
{
    printf("moved: %s %s\n", name, new_name);
    return NULL;
}

#if 0
void f2p_iter_new(f2p_file_iter_t* iter)
{

}

file_t* f2p_iter_next(f2p_file_iter_t* iter)
{

    return NULL;
}
#endif

int f2p_file_remap(
    f2p_t* me_,
    char* name,
    unsigned int piece_idx,
    unsigned long pieces)
{

    return 0;
}

void* f2p_get_files(f2p_t* me_)
{
    f2p_private_t* me = (void*)me_;

    return me->files;
}

void* f2p_get_file_from_path(f2p_t* me_, const char* path)
{
    f2p_private_t* me = (void*)me_;

    return hashmap_get(me->files, path);
}

#if 0
int f2p_get_file_from_path_len(f2p_t* me_,
        const char* path, unsigned int len)
{

}
#endif

