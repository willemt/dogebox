#ifndef DOGEBOX_MSGHANDLER_PRIVATE_H
#define DOGEBOX_MSGHANDLER_PRIVATE_H

int of_pwp_filelog(pwp_msghandler_private_t *me, msg_t* m, void* udata,
        const unsigned char** buf, unsigned int *len);

int of_pwp_piecelog(pwp_msghandler_private_t *me, msg_t* m, void* udata,
        const unsigned char** buf, unsigned int *len);

#endif /* DOGEBOX_MSGHANDLER_PRIVATE_H */
