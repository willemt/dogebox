#ifndef DOGEBOX_H
#define DOGEBOX_H

#define PROTOCOL_NAME "onefolder20140110"

enum {
    /**
     * A listing of all files we have */
    OF_MSGTYPE_FILELOG = 9,

    /**
     * A listing of all pieces we have */
    OF_MSGTYPE_PIECELOG = 10,

    /**
     * Indicate to our peer that we don't have this piece anymore */
    OF_MSGTYPE_DONTHAVE = 11,

    /**
     * Indicate to our peer that we don't have this piece anymore */
    OF_MSGTYPE_ACTIONLOG = 12,
};

#if 0
typedef struct {
    /* protocol name */
    int pn_len;
    unsigned char* pn;
    //unsigned char* reserved;
    //unsigned char* infohash;
    //unsigned char* peerid;
} of_handshake_t;
#endif

typedef struct {
    int filelog_len;
    int piecelog_len;
    char* filelog;
    char* piecelog;
} msg_fulllog_t;

typedef struct {
    int payload_len;
    unsigned char* payload;
} msg_pwp_t;

#if 0
int of_handshaker_send_handshake(
        void* callee,
        void* udata,
        int (*send)(void *callee, const void *udata, const void *send_data, const int len));

void* of_handshaker_new(unsigned char* expected_info_hash, unsigned char* mypeerid);

void of_handshaker_release(void* hs);

int of_handshaker_dispatch_from_buffer(void* me_,
        const unsigned char** buf, unsigned int* len);
#endif

#endif /* DOGEBOX_H */
