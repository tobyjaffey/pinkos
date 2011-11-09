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
#include "shell.h"
#include "console.h"
#include "parse.h"
#include "led.h"
#include "radio.h"
#include "watchdog.h"
#include "clock.h"
#include "packet.h"

#include <string.h>

/* maximum number of command line arguments */
#define MAXARGS 5

static data const char *argv[MAXARGS];
static uint8_t argc = 0;

static void cmd_reset(void)
{
    watchdog_cpureset();
}

static void cmd_boot(void)
{
    /* disable global interrupts */
    EA = 0;

    /* clear interrupt status flags */
    TCON = 0x05;    /* reserved bits */
    S0CON = 0x00;
    S1CON = 0x00;
    IRCON = 0x00;
    IRCON2 = 0x00;

    /* disable interrupts */
    IEN0 = 0x00;
    IEN1 = 0x00;
    IEN2 = 0x00;

    /* direct interrupts to the app */
    F1 = 0;

    /* down the rabbit hole */
_asm
    clr a
    mov ie, a
    mov ip, a
    mov psw, a
    mov dpl, a
    mov dph, a
    mov r7, a
    mov r6, a
    mov r5, a
    mov r4, a
    mov r3, a
    mov r2, a
    mov r1, a
    mov r0, a
    mov sp, #7
    ljmp #APPLICATION_OFFSET
_endasm;
}

/* build and send a packet */
/* tx <dst8> <seq8> <typ8> <str> */
static void cmd_tx(void)
{
    argc--;
    if (argc > 3)
    {
        uint8_t len;
        data uint8_t *buf;

        len = strlen(argv[4]);
        buf = packet_build(parse_number(argv[3]), len);
        while(*argv[4])
            *buf++ = *argv[4]++;
        packet_send(parse_number(argv[1]), parse_number(argv[2]));
    }
}

/* set a remote led */
/* teleled <dst8> <bool> */
static void cmd_teleled(void)
{
    argc--;
    if (argc == 2)
    {
        data uint8_t *buf;

        buf = packet_build(PACKET_TYPE_LED, 1);
        *buf = parse_number(argv[2]);
        packet_send(parse_number(argv[1]), PACKET_SEQ_REQ_ACK);
    }
}

#ifndef BOARD_IMME_HANDSET
/* enable graphics mode on a remote device */
/* gfxinit <dst8> */
static void cmd_gfxinit(void)
{
    argc--;
    if (argc == 1)
    {
        packet_build(PACKET_TYPE_GFXINIT, 0);
        packet_send(parse_number(argv[1]), PACKET_SEQ_REQ_ACK);
    }
}

/* plot 8x8 pixels on remote device */
/* gfxplot <dst8> <x> <y> HHHHHHHHHHHHHHHH */
static void cmd_gfxplot(void)
{
    // addr x y <hexdata*8>
    argc--;
    if (argc == 4)
    {
        data uint8_t *buf;

        buf = packet_build(PACKET_TYPE_GFXPLOTLINE, 8 + 2);
        *buf++ = parse_number(argv[2]);
        *buf++ = parse_number(argv[3]);
        parse_hexdata(argv[4], buf, 8);
        packet_send(parse_number(argv[1]), 0x00);
    }
}

/* send test pattern to remote device */
/* gfxtest <dst8> */
static void cmd_gfxtest(void)
{
    argc--;
    if (argc == 1)
    {
        data uint8_t *buf;
        uint8_t x, y;
        uint8_t dst = parse_number(argv[1]);
        const uint8_t patt[8] = {0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F};

        for (y=0;y<8;y++)
        {
            for (x=0;x<16;x++)
            {
                buf = packet_build(PACKET_TYPE_GFXPLOTLINE, 8 + 2);
                *buf++ = x << 3;
                *buf++ = y;
                *buf++ = patt[0];
                *buf++ = patt[1];
                *buf++ = patt[2];
                *buf++ = patt[3];
                *buf++ = patt[4];
                *buf++ = patt[5];
                *buf++ = patt[6];
                *buf++ = patt[7];
                packet_send(parse_number(argv[1]), 0x00);
            }
        }
    }
}
#endif

/* control local led */
/* led <bool> */
static void cmd_led(void)
{
    if (argc > 1)
    {
        switch(parse_number(argv[1]))
        {
            case 0: led_off(); return;
            case 1: led_on(); return;
            default: led_toggle(); return;
        }
    }
}

/* get/set xram */
/* xdata <addr16> [data8] */
static void cmd_xdata(void)
{
    uint16_t addr, val;

    argc--;

    if (argc < 1)
        return;

    addr = parse_number(argv[1]);

    switch(argc)
    {
        case 1: /* get */
            console_putc('*');
            console_puthex16(addr);
            console_putc('=');
            console_puthex8(*((__xdata volatile unsigned char *)addr));
            console_newline();

        /* DROP THROUGH */

        case 2: /* set */
            val = parse_number(argv[2]);
            *((__xdata volatile unsigned char *)addr) = val;
        break;

    }
}

/* get/set ram */
/* pdata <addr16> [data8] */
static void cmd_pdata(void)
{
    uint16_t addr, val;

    switch(argc)
    {
        case 1: /* get */
            if (addr = parse_number(argv[1]))
            {
                __pdata volatile unsigned char *p = (__pdata volatile unsigned char *)(addr);
                
                console_putc('*');
                console_puthex16(addr);
                console_putc('=');
                console_puthex8(*p);
                console_newline();
            }
            else
            {
                return;
            }

        /* DROP THROUGH */

        case 2: /* set */
            if (val = parse_number(argv[2]))
            {
                __pdata volatile unsigned char *p = (__pdata volatile unsigned char *)(val);
                *p = val;
            }
        break;

    }
}

void shell_exec(data char *line)
{
    char c;

    /* fill out argc and argv */
    argc = 0;
    argv[argc++] = line;
    while ((c = *line++) != 0 && argc < MAXARGS)
    {
        if (' ' == c)
        {
            *(line-1) = 0;
            argv[argc++] = line;
        }
    }

    if (0 == strcmp(*argv, "help"))
    {
        console_puts("PinkOS r");
        console_putdec(BUILD_VERSION);
        console_puts(" address 0x");
        console_puthex8(ADDR);
        console_newline();
        console_puts("Joby Taffey <jrt@hodgepig.org>");
        console_newline();
        console_puts("reset");
        console_newline();
        console_puts("<xdata|pdata> <addr16> [val8]");
        console_newline();
        console_puts("led <bool>");
        console_newline();
        console_puts("tx <dst8> <seq8> <typ8> <str>");
        console_newline();
        console_puts("teleled <dst8> <bool>");
        console_newline();
    }
    else
    if (0 == strcmp(*argv, "reset"))
        cmd_reset();
    else
    if (0 == strcmp(*argv, "boot"))
        cmd_boot();
    else
    if (0 == strcmp(*argv, "led"))
        cmd_led();
    else
    if (0 == strcmp(*argv, "xdata"))
        cmd_xdata();
    else
    if (0 == strcmp(*argv, "pdata"))
        cmd_pdata();
    else
    if (0 == strcmp(*argv, "tx"))
        cmd_tx();
    else
    if (0 == strcmp(*argv, "teleled"))
        cmd_teleled();
#ifndef BOARD_IMME_HANDSET
    else
    if (0 == strcmp(*argv, "gfxinit"))
        cmd_gfxinit();
    else
    if (0 == strcmp(*argv, "gfxplot"))
        cmd_gfxplot();
    else
    if (0 == strcmp(*argv, "gfxtest"))
        cmd_gfxtest();
#endif
}

