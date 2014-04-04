
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

#include "file2piece_mapper.h"
#include "linked_list_hashmap.h"

#include "linked_list_queue.h"

#include "bt.h"
#include "bt_piece_db.h"

typedef struct piecerange_s piecerange_t;

struct piecerange_s {
    int idx;
    int npieces;
    piecerange_t *prev, *next;
    file_t *f;
};

typedef struct {
    hashmap_t *files;
    void* piecedb;
    unsigned int piece_size;

    /* TODO: replace with skip list */
    /* linked list of piece ranges */
    piecerange_t* prange;
} f2p_private_t;

/**
 * djb2 by Dan Bernstein. */
static unsigned long __file_hash(const void *obj)
{
    const char* str;
    unsigned long hash = 5381;
    int c;
    
    for (str = obj; c = *str++;)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static long __file_compare(const void *obj, const void *other)
{
    const char* f1 = obj;
    const char* f2 = other;
    return strcmp(f1, f2);
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
    if (0 == size)
        return 1;

    return (size / piece_size) + (0 == size % piece_size ? 0 : 1);
}

unsigned int f2p_pieces_required_for_filesize(f2p_t* me_, unsigned int size)
{
    f2p_private_t* me = (void*)me_;
    return __pieces_required(size, me->piece_size);
}

static void __add_piecerange(f2p_private_t* me, piecerange_t* n)
{
    if (!me->prange || n->idx < me->prange->idx)
    {
        n->next = me->prange;
        n->prev = NULL;
        me->prange = n;
        if (n->next)
            n->next->prev = n;
        return;
    }

    piecerange_t* p;

    for (p = me->prange; p; p = p->next)
    {
        piecerange_t* o = p->next;

        if (!o || n->idx < o->idx)
            break;
    }

    n->next = p->next;
    n->prev = p;
    p->next = n;
    if (n->next)
        n->next->prev = n;
}

piecerange_t* __new_piecerange(int idx, int npieces, file_t* f)
{
    piecerange_t* pr = malloc(sizeof(piecerange_t));
    pr->idx = idx;
    pr->npieces = npieces;
    pr->f = f;
    return pr;
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

    //printf("added: %s %dB %d\n", name, size, is_dir);

    file_t* f;
        
    f = f2p_get_file_from_path(me_, name);

    if (f)
        return NULL;

    f = calloc(1, sizeof(file_t));
    f->path = strdup(name);
    f->path_len = strlen(name);
    f->is_dir = is_dir;
    f->size = size;
    f->mtime = mtime;
    hashmap_put(me->files, f->path, f);

    npieces = __pieces_required(size, me->piece_size);
    idx = bt_piecedb_add(me->piecedb, npieces);

    piecerange_t* pr = __new_piecerange(idx,npieces,f);
    __add_piecerange(me,pr);

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

    file_t* f;

    if (!(f = f2p_get_file_from_path(me_, name)))
        return NULL;

    f->is_deleted = 1;
    return f;
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

static piecerange_t* __remove_piecerange(f2p_private_t* me, piecerange_t* pr)
{
    if (pr->prev)
        pr->prev->next = pr->next;
    else me->prange = NULL;
    if (pr->next)
        pr->next->prev = pr->prev;
    return pr;
}

void* f2p_file_remap(
    f2p_t* me_,
    char* name,
    const unsigned int idx)
{
    f2p_private_t* me = (void*)me_;
    file_t* f;
    int i;
        
    if (!(f = f2p_get_file_from_path(me_, name)))
        return NULL;

    piecerange_t* p;
    int npieces = __pieces_required(f->size, me->piece_size);
    for (p = me->prange; p; p = p->next)
    {
        if (p->f == f)
            free(__remove_piecerange(me, p));
    }
    for (i=f->piece_start; i<f->piece_start + npieces; i++)
    {
        bt_piecedb_remove(me->piecedb, i);
    }

    linked_list_queue_t *removals = llqueue_new();
    for (p = me->prange; p; p = p->next)
    {
        if (p->idx <= idx + npieces && idx <= p->idx + p->npieces)
        {
            __remove_piecerange(me, p);
            int new_idx = bt_piecedb_add(me->piecedb, p->npieces);
            __add_piecerange(me, __new_piecerange(new_idx, p->npieces, f));
            llqueue_offer(removals, p);
        }
    }

    while ((p = llqueue_poll(removals)))
    {
        for (i=p->idx; i < p->idx + p->npieces; i++)
            bt_piecedb_remove(me->piecedb, i);
        free(p);
    }
    llqueue_free(removals);

    int new_idx = bt_piecedb_add_at_idx(me->piecedb, npieces, idx);
    __add_piecerange(me, __new_piecerange(idx, npieces, f));
    assert(new_idx == idx);


#if 0
    if (bt_piecedb_get(me->piecedb, idx))
    {
        
    }
#endif

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

void* f2p_get_files_from_piece_idx(f2p_t* me_, int idx)
{
    f2p_private_t* me = (void*)me_;
    file_t* f;
    piecerange_t* p;

    for (p = me->prange;
            p && idx < p->idx && p->idx + p->npieces < idx;
            p = p->next);

    if (!p || idx < p->idx || p->idx + p->npieces < idx)
        return NULL;

    return p->f;
}

int f2p_get_nfiles(f2p_t* me_)
{
    f2p_private_t* me = (void*)me_;
    return hashmap_count(me->files);
}

#if 0
int f2p_get_file_from_path_len(f2p_t* me_,
        const char* path, unsigned int len)
{

}
#endif

