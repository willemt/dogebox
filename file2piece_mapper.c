
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

#include "file2piece_mapper.h"
#include "linked_list_hashmap.h"

typedef struct {
    hashmap_t *files;
} f2p_private_t;

typedef struct file_s {
    char* path;
    unsigned int size;
    unsigned int mtime;
    int is_dir;
    void* udata;
    /* piece index */
    int piece_start;
    /* number of pieces */
    int pieces;
} file_t;

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

f2p_t* f2p_new()
{
    f2p_private_t* me;
    me = calloc(1,sizeof(f2p_private_t));
    me->files = hashmap_new(__file_hash, __file_compare, 11);
    return NULL;
}

int f2p_file_added(
    f2p_t* me_,
    char* name,
    int is_dir,
    unsigned int size,
    unsigned long mtime)
{
    printf("added: %s %dB %d\n", name, size, is_dir);
    return 0;
}

int f2p_file_removed(
    f2p_t* me_,
    char* name)
{
    printf("removed: %s\n", name);
    return 0;
}

int f2p_file_changed(
    f2p_t* me_, char* name, int new_size, unsigned long mtime)
{
    printf("changed: %s %d\n", name, new_size);
    return 0;
}

int f2p_file_moved(
    f2p_t* me_, char* name, char* new_name, unsigned long mtime)
{
    printf("moved: %s %s\n", name, new_name);
    return 0;
}

