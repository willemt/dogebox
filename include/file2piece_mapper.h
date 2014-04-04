#ifndef FILE2PIECE_MAPPER_H_
#define FILE2PIECE_MAPPER_H_

typedef void* f2p_t;

typedef struct {
    void* data;
} f2p_file_iter_t;

typedef struct {
    int pass;
} f2p_cbs_t;

typedef struct file_s {
    char* path;
    int path_len;

    /* size of file in bytes */
    unsigned int size;

    /* last time the file was modified */
    unsigned int mtime;

    int is_dir;
    void* udata;
    /* piece index */
    int piece_start;
    /* number of pieces */
    //int npieces;

    int is_deleted;
} file_t;

f2p_t* f2p_new(void* piecedb, unsigned int piece_size);

/**
 * Will add required pieces to piece database
 * @return file added; NULL if file already exists */
void* f2p_file_added(
    f2p_t* me_,
    char* name,
    int is_dir,
    unsigned int size,
    unsigned long mtime);

/**
 * @return file removed */
void* f2p_file_removed(f2p_t* me_, char* name);

/**
 * @return file changed */
void* f2p_file_changed(
    f2p_t* me_, char* name, int new_size, unsigned long mtime);

/**
 * @return file changed */
void* f2p_file_moved(
    f2p_t* me_, char* name, char* new_name, unsigned long mtime);

/**
 * @return file changed */
void* f2p_file_remap(
    f2p_t* me_, char* name, unsigned int piece_idx);

/**
 * @return hashmap of files */
void* f2p_get_files(f2p_t* me_);

void* f2p_get_file_from_path(f2p_t* me_, const char* path);

/**
 * This version doesn't assume \0 terminated strings */
int f2p_get_file_from_path_len(f2p_t* me_,
        const char* path, unsigned int len);

/**
 * @return number of files */
int f2p_get_nfiles(f2p_t* me_);

/**
 * @return number of pieces required for file of this size */
unsigned int f2p_pieces_required_for_filesize(f2p_t* me_, unsigned int size);

void* f2p_get_files_from_piece_idx(f2p_t* me_, int idx);

#endif /* FILE2PIECE_MAPPER_H_ */
