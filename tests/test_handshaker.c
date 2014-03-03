
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "CuTest.h"

#include "bitfield.h"
#include "dogebox.h"
#include "dogebox_connection.h"
#include "dogebox_handshaker.h"
#include "bitstream.h"

static char* __mock_infohash = "abcdef12345678900000";
static char* __mock_their_peer_id = "00000000000000000000";
static char* __mock_my_peer_id = "00000000000000000001";

void Testof_handshake_disconnect_if_handshake_has_invalid_name_length(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    /* handshake */
    bitstream_write_ubyte(&ptr, 0); /* pn len */

    /* setup */
    hs = of_handshaker_new((unsigned char*)__mock_infohash, (unsigned char*)__mock_my_peer_id);

    /* receive */
    len = 1;
    ret = of_handshaker_dispatch_from_buffer(hs, (const unsigned char**)&m, &len);
    CuAssertTrue(tc, -1 == ret);
}

void Testof_handshake_disconnect_if_handshake_has_invalid_protocol_name(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    /* handshake */
    bitstream_write_ubyte(&ptr, strlen("Garbage Protocol")); /* pn len */
    bitstream_write_string(&ptr, "Garbage Protocol", strlen("Garbage Protocol")); /* pn */

    /* setup */
    hs = of_handshaker_new((unsigned char*)__mock_infohash, (unsigned char*)__mock_my_peer_id);

    /* receive */
    len = 1 + strlen("Garbage Protocol") + 8 + 20 + 20;
    ret = of_handshaker_dispatch_from_buffer(hs, (const unsigned char**)&m, &len);
    CuAssertTrue(tc, -1 == ret);
}

void Testof_handshake_success_from_good_handshake(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;

    /* handshake */
    bitstream_write_ubyte(&ptr, strlen(PROTOCOL_NAME)); /* pn len */
    bitstream_write_string(&ptr, PROTOCOL_NAME, strlen(PROTOCOL_NAME)); /* pn */

    /* setup */
    hs = of_handshaker_new((unsigned char*)__mock_infohash,
            (unsigned char*)__mock_my_peer_id);

    /* receive */
    len = 1 + strlen(PROTOCOL_NAME);
    ret = of_handshaker_dispatch_from_buffer(hs, (const unsigned char**)&m, &len);
    CuAssertTrue(tc, 1 == ret);
}

typedef struct {
    unsigned char* data;
    int len;
} faux_msg_t;

static int __faux_send(
    void *callee,
    const void *udata,
    const void *send_data,
    const int len)
{
    faux_msg_t *m = callee;
    memcpy(m->data,send_data,len);
    m->len = len;
    return 1;
}

void Testof_handshake_sent_handshake_is_a_good_handshake(
    CuTest * tc
)
{
    void *hs;
    unsigned char msg[1000], *ptr = msg, *m = msg;
    unsigned int ii, ret, len;
    faux_msg_t fm;

    fm.data = msg;
    fm.len = 0;

    of_send_handshake(&fm, NULL, __faux_send, NULL, NULL);

    /* setup */
    hs = of_handshaker_new(
            (unsigned char*)__mock_infohash,
            (unsigned char*)__mock_my_peer_id);

    /* receive */
    len = 1 + strlen(PROTOCOL_NAME);
    ret = of_handshaker_dispatch_from_buffer(hs, (const unsigned char**)&m, &len);
    CuAssertTrue(tc, 1 == ret);
}

