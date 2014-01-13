
#define PROTOCOL_NAME "onefolder20140110"
enum {
    OF_MSGTYPE_KEEPALIVE = 0,
    OF_MSGTYPE_FULLLOG = 1,
    OF_MSGTYPE_BT = 2,
};

typedef struct {
    /* protocol name */
    int pn_len;
    unsigned char* pn;
    //unsigned char* reserved;
    //unsigned char* infohash;
    //unsigned char* peerid;
} of_handshake_t;


typedef struct {
    int filelog_len;
    int piecelog_len;
    char* filelog;
    char* piecelog;
} msg_fulllog_t;

int of_handshaker_send_handshake(
        void* callee,
        void* udata,
        int (*send)(void *callee, const void *udata, const void *send_data, const int len));

void* of_handshaker_new(unsigned char* expected_info_hash, unsigned char* mypeerid);

void of_handshaker_release(void* hs);

of_handshake_t* of_handshaker_get_handshake(void* me_);

int of_handshaker_dispatch_from_buffer(void* me_,
        const unsigned char** buf, unsigned int* len);
