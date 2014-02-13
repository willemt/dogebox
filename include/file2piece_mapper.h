
typedef void* f2p_t;

typedef struct {


} f2p_cbs_t;

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
