
#ifndef FILE2PIECE_MAPPER_H_
#define FILE2PIECE_MAPPER_H_

typedef void* f2p_t;

typedef struct {
    void* data;
} f2p_file_iter_t;

typedef struct {


} f2p_cbs_t;

typedef struct file_s {
    char* path;
    unsigned int size;
    unsigned int mtime;
    int is_dir;
    void* udata;
    /* piece index */
    int piece_start;
    /* number of pieces */
    int npieces;
} file_t;

f2p_t* f2p_new(void* piecedb, unsigned int piece_size);

int f2p_file_added(
    f2p_t* me_,
    char* name,
    int is_dir,
    unsigned int size,
    unsigned long mtime);

int f2p_file_removed(
    f2p_t* me_,
    char* name);

int f2p_file_changed(
    f2p_t* me_, char* name, int new_size, unsigned long mtime);

int f2p_file_moved(
    f2p_t* me_, char* name, char* new_name, unsigned long mtime);

/**
 * @return hashmap of files */
void* f2p_get_files(f2p_t* me_);

#endif /* FILE2PIECE_MAPPER_H_ */
