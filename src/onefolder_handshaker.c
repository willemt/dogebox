
/**
 * Copyright (c) 2011, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. 
 *
 * @file
 * @brief Manage a connection with a peer
 * @author  Willem Thiart himself@willemthiart.com
 * @version 0.1
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "onefolder_handshaker.h"

#include "bitfield.h"
#include "bitstream.h"

typedef struct {
    of_handshake_t hs;
    unsigned int bytes_read;

    unsigned char* cur;
    unsigned char* curr_value;
} of_handshaker_t;

int of_handshaker_send_handshake(
        void* callee,
        void* udata,
        int (*send)(void *callee,
            const void *udata,
            const void *send_data,
            const int len),
        char* expected_ih,
        char* my_pi)
{
    char buf[1024], *prot_name = PROTOCOL_NAME, *ptr;
    int size, ii;

    assert(NULL != expected_ih);
    assert(NULL != my_pi);

    ptr = buf;

    /* protocol name length */
    bitstream_write_ubyte((unsigned char**)&ptr, strlen(prot_name));

    /* protocol name */
    bitstream_write_string((unsigned char**)&ptr, prot_name, strlen(prot_name));

    size = 1 + strlen(prot_name);// + 8 + 20 + 20;

    if (0 == send(callee, udata, buf, size))
    {
//        __log(me, "send,handshake,fail");
        return 0;
    }

    return 1;
}

void* of_handshaker_new(unsigned char* expected_info_hash, unsigned char* mypeerid)
{
    of_handshaker_t* me;

    me = calloc(1,sizeof(of_handshaker_t));
//    me->expected_ih = expected_info_hash;
//    me->my_pi = mypeerid;
    return me;
}

void of_handshaker_release(void* hs)
{
    of_handshaker_t* me = hs;

    free(me);
}

of_handshake_t* of_handshaker_get_handshake(void* me_)
{
    of_handshaker_t* me = me_;

#if 0
    if (me->status == 1)
        return &me->hs;
    return NULL;
#endif
        return &me->hs;
}

unsigned char __readbyte(unsigned int* bytes_read, const unsigned char **buf, unsigned int* len)
{
    unsigned char val;

    val = **buf;
    *buf += 1;
    *bytes_read += 1;
    *len -= 1;
    return val;
}

int of_handshaker_dispatch_from_buffer(void* me_,
        const unsigned char** buf, unsigned int* len)
{
    of_handshaker_t* me = me_;
    of_handshake_t* hs = &me->hs;

    while (0 < *len)
    {

    /* protcol name length
     * The unsigned value of the first byte indicates the length of a
     * character string containing the prot name. In BTP/1.0 this number
     * is 19. The local peer knows its own prot name and hence also the
     * length of it. If this length is different than the value of this
     * first byte, then the connection MUST be dropped. */
        if (me->curr_value == NULL)
        {
            hs->pn_len = __readbyte(&me->bytes_read, buf, len);

            if (0 == hs->pn_len)
            {
                printf("ERROR: invalid length\n");
                return -1;
            }

            me->cur = me->curr_value = hs->pn = malloc(hs->pn_len);
        }
    /* protocol name
    This is a character string which MUST contain the exact name of the 
    prot in ASCII and have the same length as given in the Name Length
    field. The prot name is used to identify to the local peer which
    version of BTP the remote peer uses. If this string is different
    from the local peers own prot name, then the connection is to be
    dropped. */
        else if (me->curr_value == hs->pn)
        {
            *me->cur = __readbyte(&me->bytes_read, buf, len);
            me->cur++;

            /* validate */
            if (me->cur - me->curr_value == hs->pn_len)
            {
                if (0 != strncmp((char*)hs->pn, PROTOCOL_NAME,
                    hs->pn_len < strlen(PROTOCOL_NAME) ?
                        hs->pn_len : strlen(PROTOCOL_NAME)))
                {
                    printf("ERROR: incorrect protocol name\n");
                    return -1;
                }

                me->cur = me->curr_value = hs->reserved = malloc(8);
                return 1;
            }
        }
        else
        {
            printf("ERROR: invalid handshake\n");
            return -1;
        }
    }

    return 0;
}

