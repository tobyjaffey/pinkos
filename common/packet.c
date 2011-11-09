/*
 * This file is part of PinkOS
 * Copyright (c) 2010, Joby Taffey <jrt@hodgepig.org>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"
#include "radio.h"
#include "console.h"
#include "packet.h"
#include "clock.h"
#include "led.h"
#include <string.h>
#include <ctype.h>

#define DEBUG 1

/* acknowledge packet if req_ack bit set */
static void packet_autoack(void)
{ 
    data packet_t *pkt = (data packet_t *)radio_getbuf();

    if (pkt->dstAddr != BROADCAST_ADDR && (pkt->seq & PACKET_SEQ_REQ_ACK))
    {
        /* turn off ack bit, change length for empty payload, send it back */
        pkt->dstAddr = pkt->srcAddr;
        pkt->srcAddr = ADDR;
        pkt->seq &= ~PACKET_SEQ_REQ_ACK;
        pkt->type = PACKET_TYPE_ACK;
        pkt->len = sizeof(packet_t)-1;  /* length byte not included */
        radio_tx();
    }
}

#ifdef DEBUG
/* dump contents of the buffer */
static void packet_dump(void)
{
    const data uint8_t *pktbuf = radio_getbuf();
    const data packet_t *pkt = (const data packet_t *)pktbuf;
    const data uint8_t *p;
    uint8_t payload_len = pkt->len - (sizeof(packet_t)-1);
    uint8_t i;

    console_puts("RX");
    console_puts(" len="); console_puthex8(pkt->len);
    console_puts(" dst="); console_puthex8(pkt->dstAddr);
    console_puts(" src="); console_puthex8(pkt->srcAddr);
    console_puts(" seq="); console_puthex8(pkt->seq & PACKET_SEQ_MASK);
    console_puts(" reqack="); console_putc(pkt->seq & PACKET_SEQ_REQ_ACK ? '1' : '0');
    console_puts(" rssi="); console_puthex8(RSSI);
    console_puts(" lqi="); console_putdec(LQI & 0x7F);

    console_puts(" type="); console_puthex8(pkt->type);

    console_putc(' ');
    p = pktbuf + sizeof(packet_t);
    for (i=0;i<payload_len;i++)
        console_puthex8(p[i]);

    console_puts(" \"");
    p = pktbuf + sizeof(packet_t);
    for (i=0;i<payload_len;i++)
        console_putc(isalnum(p[i]) ? p[i] : '.');
    console_puts("\"");

    console_newline();
}
#endif

/* begin building a packet */
data uint8_t *packet_build(uint8_t type, uint8_t len)
{
    data uint8_t *pktbuf = radio_getbuf();
    data packet_t *pkt = (data packet_t *)pktbuf;

    pkt->len = (sizeof(packet_t)-1) + len;  /* length byte not included in count */
    pkt->srcAddr = ADDR;
    pkt->type = type;

    return pktbuf + sizeof(packet_t);
}

/* send packet (blocks) */
void packet_send(uint8_t dstAddr, uint8_t seq)
{
    data packet_t *pkt = (data packet_t *)radio_getbuf();

    pkt->dstAddr = dstAddr;
    pkt->seq = seq;

    if (radio_tx())
        console_puts("OK");
    else
        console_puts("FAIL");
    console_newline();

    clock_delayms(10);
}

static void packet_handle(void)
{
    const data uint8_t *pktbuf = radio_getbuf();
    const data packet_t *pkt = (const data packet_t *)pktbuf;
    uint8_t payload_len = pkt->len - (sizeof(packet_t)-1);

    packet_rx_callback(pkt->srcAddr, pkt->seq & PACKET_SEQ_MASK, pkt->type, pktbuf + sizeof(packet_t), payload_len);
}


void packet_process(void)
{
#ifdef DEBUG
    packet_dump();
#endif
    packet_handle();

    /* this modifies the buffer, do it last */
    packet_autoack();
}


