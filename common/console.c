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
#include "console.h"
#ifdef BOARD_IMME_DONGLE
#include "uart0.h"
#endif
#ifdef BOARD_IMME_HANDSET
#include "lcdterm.h"
#endif
#include "shell.h"

/* line mode or key (getch) mode */
static bool linemode;

/* line mode buffer */
static data char linebuf[LINEBUF_MAX];
static data uint8_t linebuf_tail;
static bool got_eol = false;            /* received end of line */

/* key mode buffer */
static bool got_key = false;
static data uint8_t last_key = 0x00;   // last character received

/* pending response to control characters */
enum
{
    PENDING_NONE = 0,
    PENDING_BELL,
    PENDING_CHAR,
    PENDING_BACKSPACE,
    PENDING_NEWLINE,
};
static volatile data uint8_t pending = PENDING_NONE;


void console_prompt(void)
{
    console_puts(">");
}

void console_init(void)
{
    console_set_linemode(true);
}

void console_putc(uint8_t c)
{
#ifdef BOARD_IMME_DONGLE
    uart0_putc(c);
#endif
#ifdef BOARD_IMME_HANDSET
    lcdterm_putc(c);
#endif
}

void console_newline(void)
{
#ifdef BOARD_IMME_DONGLE
    console_putc('\r');
    console_putc('\n');
#endif
#ifdef BOARD_IMME_HANDSET
    lcdterm_newline();
#endif
}

void console_set_linemode(bool on)
{
    linemode = on;

    if (on)
    {
        linebuf_tail = 0;
        got_eol = false;
    }
    else
    {
        got_key = false;
    }
}

/* in key mode, poll for keypress */
bool console_key_poll(uint8_t *c)
{
    if (got_key)
    {
        *c = last_key;
        got_key = false;
        return true;
    }
    return false;
}

/* is console ready to receive? */
bool console_rx_ready_callback(void)
{
    if (linemode)
        return !got_eol;
    else
        return true;
}

/* write pending control character responses */
static void handle_pending(void)
{
    switch(pending)
    {
        case PENDING_BELL:
        console_putc('\a');
        break;

        case PENDING_NEWLINE:
        console_newline();
        break;

        case PENDING_CHAR:
        console_putc(linebuf[linebuf_tail-1]);
        break;

        case PENDING_BACKSPACE:
        console_putc('\b');
        console_putc(' ');
        console_putc('\b');
        break;

        default:
        case PENDING_NONE:
        break;
    }
    pending = PENDING_NONE;
}

/* receive a character, called under interrupt */
void console_rx_callback(uint8_t c)
{
    if (!linemode)
    {
        got_key = true;
        last_key = c;
    }
    else
    {
        if (got_eol) /* throw away characters until line is processed */
            return;

        switch(c)
        {
            case 0x0D:
                got_eol = true;
                pending = PENDING_NEWLINE;
                break;
            case '\b': /* backspace */
            case 0x7F: /* delete */
                if (linebuf_tail > 0)
                {
                    linebuf_tail--;
                    pending = PENDING_BACKSPACE;
                }
                break;
            default:
                if (linebuf_tail < sizeof(linebuf)-1)
                {
                    pending = PENDING_CHAR;
                    linebuf[linebuf_tail++] = c;
                }
                else
                {
                    pending = PENDING_BELL;
                }
                break;
        }
    }

    /* FIXME shouldn't be doing this in ISR */
    handle_pending();
}

/* check for complete line buffer and execute */
void console_tick(void)
{
    if (linemode)
    {
        if (got_eol)
        {
            if (linebuf_tail > 0)
            {
                linebuf[linebuf_tail] = 0; // terminate it
                shell_exec(linebuf);
            }
            linebuf_tail = 0;
            console_prompt();
            got_eol = false;
        }
    }
}

void console_puts(uint8_t *str)
{
    while(*str)
        console_putc(*str++);
}

static char nibble_to_char(uint8_t nibble)
{
    if (nibble < 0xA)
        return nibble + '0';
    return nibble - 0xA + 'A';
}

void console_puthex8(uint8_t h)
{
    console_putc(nibble_to_char((h & 0xF0)>>4));
    console_putc(nibble_to_char(h & 0x0F));
}

void console_puthex16(uint16_t h)
{
    console_puthex8((h & 0xFF00) >> 8);
    console_puthex8(h & 0xFF);
}

void console_puthex32(uint32_t h)
{
    console_puthex8((h & 0xFFFFFF00) >> 24);
    console_puthex8((h & 0x00FF0000) >> 16);
    console_puthex8((h & 0x0000FF00) >> 8);
    console_puthex8( h & 0x000000FF);
}

void console_putdec(uint32_t n)
{
    uint32_t m;
    bool in_leading_zeroes = true;

    for (m = 1000000000; m != 1; m/=10)
    {
        if ((n / m) != 0)
            in_leading_zeroes = false;
        if (!in_leading_zeroes)
            console_putc(nibble_to_char(n / m));
        n = n % m;
    }
    console_putc(nibble_to_char(n));
}

void console_putbin(uint8_t b)
{
    console_putc('b');
    (b & 0x80) ? console_putc('1') : console_putc('0');
    (b & 0x40) ? console_putc('1') : console_putc('0');
    (b & 0x20) ? console_putc('1') : console_putc('0');
    (b & 0x10) ? console_putc('1') : console_putc('0');
    (b & 0x08) ? console_putc('1') : console_putc('0');
    (b & 0x04) ? console_putc('1') : console_putc('0');
    (b & 0x02) ? console_putc('1') : console_putc('0');
    (b & 0x01) ? console_putc('1') : console_putc('0');
}

