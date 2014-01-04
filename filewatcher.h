
typedef struct {

    int (*file_added)(
        char* name,
        int is_dir,
        unsigned int size,
        unsigned long mtime);

    int (*file_removed)(char* name);
    
    int (*file_changed)(char* name, int new_size, unsigned long mtime);

    int (*file_moved)(char* name, char* new_name, unsigned long mtime);
    
} filewatcher_cbs_t;

typedef void filewatcher_t;

filewatcher_t* filewatcher_new(
        char* path,
        void* loop,
        filewatcher_cbs_t* cb,
        void* udata);
