#ifndef DOGEBOX_MSGHANDLER_H
#define DOGEBOX_MSGHANDLER_H

/**
 * @return new msg handler */
void* of_msghandler_new(void* pc, void* udata);

/**
 * Release memory used by message handler */
void of_msghandler_release(void *pc);

/**
 * Receive this much data.
 * If there is enough data this function will dispatch of_connection events
 * @param mh The message handler object
 * @param buf The data to be read in
 * @param len The length of the data to be read in
 * @return 1 if successful, 0 if the peer needs to be disconnected */
int of_msghandler_dispatch_from_buffer(void *mh,
        const unsigned char* buf,
        unsigned int len);


int of_pwp_filelog(void *me_, void* m_, void* udata,
        const unsigned char** buf, unsigned int *len);

int of_pwp_piecelog(void *me_, void* m_, void* udata,
        const unsigned char** buf, unsigned int *len);

#endif /* DOGEBOX_MSGHANDLER_H */
