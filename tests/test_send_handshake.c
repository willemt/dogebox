
void Testof_handshake_sent_handshake_is_a_good_handshake(
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

