
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
    char* name;
    unsigned int size;
    unsigned int mtime;
    int is_dir;
    void* udata;

    /* piece IDX */
    int piece_start;
    /* number of pieces */
    int pieces;
} file_t;


f2p_t* f2p_new()
{
    return NULL;
}

int f2p_file_added(
    char* name,
    int is_dir,
    unsigned int size,
    unsigned long mtime)
{
    printf("added: %s %dB %d\n", name, size, is_dir);
    return 0;
}

int f2p_file_removed(char* name)
{
    printf("removed: %s\n", name);
    return 0;
}

int f2p_file_changed(char* name, int new_size, unsigned long mtime)
{
    printf("changed: %s %d\n", name, new_size);
    return 0;
}

int f2p_file_moved(char* name, char* new_name, unsigned long mtime)
{
    printf("moved: %s %s\n", name, new_name);
    return 0;
}

