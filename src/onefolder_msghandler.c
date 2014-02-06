
/**
 * Copyright (c) 2011, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. 
 *
 * @file
 * @brief 
 * @author  Willem Thiart himself@willemthiart.com
 * @version 0.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* for uint32_t */
#include <stdint.h>

#include "bitfield.h"
#include "onefolder.h"
#include "onefolder_msghandler.h"

typedef struct {
//    uint32_t len;
    unsigned char id;
    unsigned int bytes_read;
    unsigned int tok_bytes_read;
    union {
        msg_fulllog_t full_log;
    };
} msg_t;

typedef struct {
    /* current message we are reading */
    msg_t msg;

    /* peer connection */
    void* pc;
} of_peer_connection_event_handler_t;

static void __endmsg(msg_t* msg)
{
    memset(msg,0,sizeof(msg_t));
}

/**
 * Flip endianess
 **/
static uint32_t fe(uint32_t i)
{
    uint32_t o;
    unsigned char *c = (unsigned char *)&i;
    unsigned char *p = (unsigned char *)&o;

    p[0] = c[3];
    p[1] = c[2];
    p[2] = c[1];
    p[3] = c[0];

    return o;
}

static int __read_uint32(
        uint32_t* in,
        msg_t *msg,
        const unsigned char** buf,
        unsigned int *len)
{
    while (1)
    {
        if (msg->tok_bytes_read == 4)
        {
            *in = fe(*in);
            msg->tok_bytes_read = 0;
            return 1;
        }
        else if (*len == 0)
        {
            return 0;
        }

        *((unsigned char*)in + msg->tok_bytes_read) = **buf;
        msg->tok_bytes_read += 1;
        msg->bytes_read += 1;
        *buf += 1;
        *len -= 1;
    }
}

/**
 * @param in Read data into 
 * @param tot_bytes_read Running total of total number of bytes read
 * @param buf Read data from
 * @param len Length of stream left to read from */
static int __read_byte(
        unsigned char* in,
        unsigned int *tot_bytes_read,
        const unsigned char** buf,
        unsigned int *len)
{
    if (*len == 0)
        return 0;

    *in = **buf;
    *tot_bytes_read += 1;
    *buf += 1;
    *len -= 1;
    return 1;
}

/**
 * create a new msg handler */
void* of_msghandler_new(void *pc)
{
    of_peer_connection_event_handler_t* me;

    me = calloc(1,sizeof(of_peer_connection_event_handler_t));
    me->pc = pc;
    return me;
}

void of_msghandler_release(void *pc)
{
    free(pc);
}

/**
 * Receive this much data.
 * If there is enough data this function will dispatch of_connection events
 * @param mh The message handler object
 * @param buf The data to be read in
 * @param len The length of the data to be read in
 * @return 1 if successful, 0 if the peer needs to be disconnected */
int of_msghandler_dispatch_from_buffer(void *mh,
        const unsigned char* buf,
        unsigned int len)
{
    of_peer_connection_event_handler_t* me = mh;
    msg_t* msg = &me->msg;

    /* while we have a stream left to read... */
    while (0 < len)
    {
        /* get message ID */
        if (msg->bytes_read < 1)
        {
            __read_byte(&msg->id, &msg->bytes_read, &buf, &len);

            switch (msg->id)
            {
            case OF_MSGTYPE_KEEPALIVE:
                break;
            case OF_MSGTYPE_FULLLOG:
                break;
            case OF_MSGTYPE_BT:
                break;
            default: assert(0); break;
            }
            __endmsg(&me->msg);
        }
        /* messages with a payload: */
        else 
        {
            switch (msg->id)
            {
            case OF_MSGTYPE_FULLLOG:
                /* get filelog_len */
                if (1 == __read_uint32(&msg->full_log.filelog_len,
                            &me->msg, &buf,&len))
                {
                    //of_conn_have(me->pc,&msg->have);
                    //__endmsg(&me->msg);
                    continue;
                }

                break;
            switch (msg->id)
            {
            case OF_MSGTYPE_BT:

                if (1 == __read_uint32(&msg->full_log.filelog_len,
                            &me->msg, &buf,&len))
                {
                    //of_conn_have(me->pc,&msg->have);
                    //__endmsg(&me->msg);
                    continue;
                }

                break;
            default:
                printf("ERROR: bad of msg type: '%d'\n", msg->id);
                return 0;
                break;
            }
        }
    }

    return 1;
}

