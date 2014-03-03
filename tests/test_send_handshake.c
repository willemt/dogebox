
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

void Testof_handshake_sent_handshake_is_a_good_handshake(
    CuTest * tc
)
{
    void *pc;
    test_sender_t sender;
    unsigned char msg[1000], *ptr;
    pwp_conn_cbs_t funcs = {
        .send = __FUNC_send,
    };
    sparsecounter_t* sc;

    /* setup */
    ptr = msg;
    __sender_set(&sender,NULL,msg);
    pc = pwp_conn_new(NULL);
    pwp_conn_set_piece_info(pc,20,20);
    pwp_conn_set_cbs(pc, &funcs, &sender);
    sc = sc_init(0);
    pwp_conn_set_progress(pc,sc);
    sc_mark_complete(sc,0,1);

    /* send msg */
    pwp_conn_send_have(pc, 17);

    /* read sent msg */
    CuAssertTrue(tc, 5 == fe(bitstream_read_uint32(&ptr)));
    CuAssertTrue(tc, PWP_MSGTYPE_HAVE == bitstream_read_ubyte(&ptr));
    CuAssertTrue(tc, 17 == fe(bitstream_read_uint32(&ptr)));

