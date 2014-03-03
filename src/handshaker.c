
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

#include "dogebox.h"
#include "dogebox_handshaker.h"

#include "bitfield.h"
#include "bitstream.h"

/* for handshaker error codes */
#include "bt.h"
//#include "pwp_handshaker.h"

typedef struct {
    of_handshake_t hs;
    unsigned int bytes_read;

    unsigned char* cur;
    unsigned char* curr_value;
} of_handshaker_t;

int of_send_handshake(
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

static unsigned char __readbyte(
        unsigned int* bytes_read,
        const unsigned char **buf,
        unsigned int* len)
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
        if (me->curr_value == NULL)
        {
            hs->pn_len = __readbyte(&me->bytes_read, buf, len);

            if (0 == hs->pn_len)
            {
                printf("ERROR: invalid length\n");
                return BT_HANDSHAKER_DISPATCH_ERROR;
            }

            me->cur = me->curr_value = hs->pn = malloc(hs->pn_len);
        }
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
                    return BT_HANDSHAKER_DISPATCH_ERROR;
                }

                //me->cur = me->curr_value = hs->reserved = malloc(8);
                return BT_HANDSHAKER_DISPATCH_SUCCESS;
            }
        }
        else
        {
            printf("ERROR: invalid handshake\n");
            return BT_HANDSHAKER_DISPATCH_ERROR;
        }
    }

    return BT_HANDSHAKER_DISPATCH_REMAINING;
}

