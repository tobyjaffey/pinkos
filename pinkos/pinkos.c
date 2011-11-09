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
#include "watchdog.h"
#include "clock.h"
#include "console.h"
#include "led.h"
#include "radio.h"
#include "packet.h"
#ifdef BOARD_IMME_HANDSET
#include "key.h"
#include "spi.h"
#include "lcd.h"
#include "lcdterm.h"
#endif
#ifdef BOARD_IMME_DONGLE
#include "uart0.h"
#endif

static void banner(void)
{
    led_on();
    clock_delayms(100);

    console_puts("PinkOS r");
    console_putdec(BUILD_VERSION);
    console_puts(" address 0x");
    console_puthex8(ADDR);
    console_newline();
    console_newline();

    led_off();
    clock_delayms(100);
}

int main(void)
{
    watchdog_init();
    clock_init();
    led_init();
#ifdef BOARD_IMME_DONGLE
    uart0_init();
#endif
#ifdef BOARD_IMME_HANDSET
    spi_init();
    key_init();
    lcdterm_init();
#endif
    radio_init(DEVADDR);

    /* Enable global interrupts handled by bootloader */
    F1 = 1;
    EA = 1;

    console_init();
    banner();
    console_prompt();

    while(1)
    {
#ifdef BOARD_IMME_HANDSET
        lcdterm_tick();
#endif
        console_tick();
        radio_tick();
    }
}

/* handle incoming radio packet */
void packet_rx_callback(uint8_t srcAddr, uint8_t seq, uint8_t type, const data uint8_t *buf, uint8_t len)
{
    switch(type)
    {
        case PACKET_TYPE_ACK:
        console_puts("ACK 0x");
        console_puthex8(seq);
        console_puts(" from 0x");
        console_puthex8(srcAddr);
        console_newline();
        break;

        case PACKET_TYPE_PUTS:
        while(len--)
            console_putc(*buf++);
        console_newline();
        break;

        case PACKET_TYPE_LED:
        if (*buf)
            led_on();
        else
            led_off();
        break;

#ifdef BOARD_IMME_HANDSET
        case PACKET_TYPE_GFXINIT:
        lcdterm_stop();
        break;

        case PACKET_TYPE_GFXPLOTLINE:
        lcd_cs(1);
        lcd_setNormalReverse(1);
        lcd_setPos(buf[0], buf[1]);
        len -= 2;   /* x,y */
        buf += 2;
        while(len--)
            lcd_txData(*buf++);
        lcd_cs(0);
        break;
#endif
    }
}



/*
 * "If you have multiple source fles in your project,
 *   interrupt service routines can be present in any of them, but a 
 *   prototype of the isr MUST be present or included in the file that
 *   contains the function main."
 *   http://sdcc.sourceforge.net/doc/sdccman.pdf
 */
#include "isr.c"



