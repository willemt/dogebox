
/**
 * Copyright (c) 2011, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. 
 *
 * @file
 * @author  Willem Thiart himself@willemthiart.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <uv.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct file_s file_t;

typedef struct {
    uv_async_t async;
    uv_mutex_t mutex;
    int waiting;
} reader_t;

uv_loop_t *loop;

struct file_s {
    char* name;
    unsigned int size;
    unsigned int mtime;
    int is_dir;
    void* udata;
};

static void readdir_cb(uv_fs_t* req);
static void stat_cb(uv_fs_t* req);

void __append(void* o, file_t* f);

int asprintf(char **resultp, const char *format, ...)
{
    char buf[1024];
    va_list args;

    va_start (args, format);
    vsprintf(buf, format, args);
    *resultp = strdup(buf);
    va_end (args);
    return 1;
}


static void stat_cb(uv_fs_t* req)
{
    file_t* f;
    reader_t* rdr;

    f = req->data;
    rdr = f->udata;

//    printf("stat %s %lluB\n", f->name, req->statbuf.st_size, req->statbuf.st_mode);

    f->size = req->statbuf.st_size;
    f->mtime = req->statbuf.st_mtim.tv_sec;
    if (req->statbuf.st_mode & S_IFDIR)
    {
        uv_fs_t *req_rd;
        int r;

        req_rd = malloc(sizeof(uv_fs_t));
        req_rd->data = rdr;
        f->is_dir = 1;
        r = uv_fs_readdir(loop, req_rd, f->name, 0, readdir_cb);
    }

    uv_mutex_lock(&rdr->mutex);
    __append(NULL, f);
    uv_mutex_unlock(&rdr->mutex);

    //assert(req->statbuf.st_size != 0);

//req->statbuf.st_atim,
//req->statbuf.st_mtim,
//req->statbuf.st_ctim,
//req->statbuf.st_birthtim,

    uv_fs_req_cleanup(req);
}

static void readdir_cb(uv_fs_t* req)
{
    int i;
    char* res;
    reader_t* rdr;

    rdr = req->data;
    res = req->ptr;

    for (i=0; i<req->result; i++)
    {
        file_t *f;
        int len;
        int r;

        f = calloc(1,sizeof(file_t));
        asprintf(&f->name, "%s/%s", req->path, res);
        
        f->udata = rdr;

        uv_fs_t *stat_req;
        stat_req = malloc(sizeof(uv_fs_t));
        //stat_req->path = f->name;
        stat_req->data = f;
        r = uv_fs_stat(loop, stat_req, f->name, stat_cb);
        res += strlen(res) + 1;
    }

    uv_fs_req_cleanup(req);
}

void __change_cb(
        uv_fs_event_t* e, const char* filename, int events, int status)
{
    printf("%d %d: %s %s\n", events, status, filename, e->filename);

    switch (events)
    {
        case UV_RENAME:
            printf("renamed: %s\n", filename);
            break;
        case UV_CHANGE:
            printf("changed: %s\n", filename);
            break;
    default:
            assert(0);
            break;
    }
}

void print_filelog(file_t* f, int depth)
{
    printf("%s %dB %llu\n", f->name, f->size, f->mtime);
}

static void __periodic(uv_timer_t* handle, int status)
{
    reader_t* r;

    r = handle->data;

    uv_mutex_lock(&r->mutex);
    printf("count: %d\n", r->waiting);
    //r->waiting -= 1;
    uv_mutex_unlock(&r->mutex);
}


void __add_file(uv_async_t *handle, int status /*UNUSED*/)
{
#if 0
    reader_t* rdr;

    printf("added\n");

    rdr = handle->data;

    print_filelog(rdr->file,0);
#endif
}

void __append(void* o, file_t* f)
{
    print_filelog(f, 0);
}

filewatcher_t* filewatcher_new(char* path, void* loop, filewatcher_t* cb, void* udata); 

#if 0
int main(int argc, char **argv)
{
    int r;
    uv_fs_t readdir_req;
    uv_fs_event_t fs_event;

    reader_t rdr;

    memset(&rdr,0,sizeof(reader_t));

    loop = uv_default_loop();

    //uv_async_init(loop, &rdr.async, __progress);
    uv_mutex_init(&rdr.mutex);

    rdr.async.data = &rdr;

#if 0
    fs_event.data = f;
    r = uv_fs_event_init(loop, &fs_event);
    r = uv_fs_event_start(&fs_event,
            __change_cb,
            ".",
            0);//UV_FS_EVENT_RECURSIVE);
#endif

    readdir_req.data = &rdr;
    r = uv_fs_readdir(loop, &readdir_req, ".", 0, readdir_cb);

    uv_timer_t *periodic_req;
    periodic_req = malloc(sizeof(uv_timer_t));
    periodic_req->data = &rdr;
    uv_timer_init(loop, periodic_req);
    uv_timer_start(periodic_req, __periodic, 0, 1000);

    uv_run(loop, UV_RUN_DEFAULT);

//    print_filelog(f);

    return 1;
}
#endif
