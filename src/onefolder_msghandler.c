
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
#include "onefolder_connection.h"
#include "onefolder_msghandler.h"

#undef min
#define min(a,b) ((a) < (b) ? (a) : (b))

typedef struct {
    int fl_read;
    int pl_read;
    msg_fulllog_t m;
} reader_fulllog_t;

typedef struct {
    int read;
    msg_pwp_t m;
} reader_pwp_t;


typedef struct {
//    uint32_t len;
    unsigned char id;
    unsigned int bytes_read;
    unsigned int tok_bytes_read;
    union {
        reader_pwp_t pwp;
        reader_fulllog_t fl;
    };
} msg_t;

typedef struct {
    /* current message we are reading */
    msg_t msg;

    /* peer connection */
    void* pc;

    /* callbacks */
    of_conn_recv_cb_t cb;
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

void* of_msghandler_new(void *pc, of_conn_recv_cb_t* cb)
{
    of_peer_connection_event_handler_t* me;

    me = calloc(1,sizeof(of_peer_connection_event_handler_t));
    me->pc = pc;
    memcpy(&me->cb,cb,sizeof(of_conn_recv_cb_t));
    return me;
}

void of_msghandler_release(void *pc)
{
    free(pc);
}

static unsigned int __increment_buf(
        unsigned int *len,
        msg_t *m,
        unsigned int size)
{
    *len -= size;
    m->bytes_read += size;
    return size;
}

int of_msghandler_dispatch_from_buffer(void *mh,
        const unsigned char* buf,
        unsigned int len)
{
    of_peer_connection_event_handler_t* me = mh;
    msg_t* m = &me->msg;

    /* while we have a stream left to read... */
    while (0 < len)
    {
        /* get message ID */
        if (m->bytes_read < 1)
        {
            __read_byte(&m->id, &m->bytes_read, &buf, &len);

            switch (m->id)
            {
            case OF_MSGTYPE_KEEPALIVE:
                me->cb.conn_keepalive(me->pc);
                __endmsg(m);
                break;
            case OF_MSGTYPE_FULLLOG:
                break;
            case OF_MSGTYPE_PWP:
                break;
            default:
                assert(0);
                break;
            }
        }
        /* messages with a payload: */
        else 
        {
            switch (m->id)
            {
            case OF_MSGTYPE_FULLLOG:
                if (m->bytes_read < 1 + 4)
                {
                    if (1 == __read_uint32(&m->fl.m.filelog_len, m, &buf,&len))
                    {

                    }
                }
                else if (m->bytes_read < 1 + 4 + 4)
                {
                    if (1 == __read_uint32(&m->fl.m.piecelog_len, m, &buf,&len))
                    {
                        // TODO: get rid of malloc()s please
                        m->fl.m.filelog = malloc(m->fl.m.filelog_len);
                        m->fl.m.piecelog = malloc(m->fl.m.piecelog_len);
                    } 
                }
                else if (m->bytes_read < 1 + 4 + 4 + m->fl.m.filelog_len)
                {
                    /* check it isn't bigger than what the message tells
                     * us we should be expecting */
                    int size = min(len, m->fl.m.filelog_len - m->fl.fl_read);

                    // TODO: get rid of strcpy() please
                    strncpy(m->fl.m.filelog + m->fl.fl_read, buf, size);
                    m->fl.fl_read += size;
                    buf += __increment_buf(&len,m,size);
                }
                else if (m->bytes_read < 1 + 4 + 4 +
                        m->fl.m.filelog_len +
                        m->fl.m.piecelog_len)
                {
                    int size = min(len, m->fl.m.piecelog_len - m->fl.pl_read);

                    strncpy(m->fl.m.piecelog + m->fl.pl_read, buf, size);
                    m->fl.pl_read += size;
                    buf += __increment_buf(&len,m,size);

                    if (m->bytes_read == 1 + 4 + 4 +
                        m->fl.m.filelog_len +
                        m->fl.m.piecelog_len)
                    {
                        me->cb.conn_fulllog(me->pc, &m->fl.m);
                        __endmsg(m);
                    }
                }
                else assert(0);
                break;

            case OF_MSGTYPE_PWP:

                if (m->bytes_read < 1 + 4)
                {
                    if (1 == __read_uint32(&m->pwp.m.payload_len, m, &buf, &len))
                    {

                    }
                }
                else
                {
                    /* check it isn't bigger than what the message tells
                     * us we should be expecting */
                    int size = min(len, m->pwp.m.payload_len - m->pwp.read);
                    me->cb.conn_pwp_dispatch(me->pc, buf, m->pwp.m.payload_len);
                    m->pwp.read += size;
                    buf += __increment_buf(&len,m,size);
                }
                
                break;

            default:
                printf("ERROR: bad of msg type: '%d'\n", m->id);
                return 0;
            }
        }
    }

    return 1;
}

