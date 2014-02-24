
#define PROTOCOL_NAME "one-folder protocol"

typedef struct {
    /* protocol name */
    int pn_len;
    unsigned char* pn;
    unsigned char* reserved;
    unsigned char* infohash;
    unsigned char* peerid;
} of_handshake_t;

/**
 * Send the handshake
 *
 * @return 0 on failure; 1 otherwise */
int of_handshaker_send_handshake(
        void* callee,
        void* udata,
        int (*send)(void *callee, const void *udata, const void *send_data, const int len),
        char* expected_ih,
        char* my_pi);


void* of_handshaker_new(
        unsigned char* expected_info_hash,
        unsigned char* mypeerid);

void of_handshaker_release(void* hs);

/**
 *  Receive handshake from other end
 *  Disconnect on any errors
 *
 *  @return 1 on succesful handshake; 0 on unfinished reading; -1 on failed handshake */
int of_handshaker_dispatch_from_buffer(void* me_,
        const unsigned char** buf, unsigned int* len);

/**
 * @return null if handshake was successful */
of_handshake_t* of_handshaker_get_handshake(void* me_);

