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
#include "lcdterm.h"
#include "lcd.h"
#include "tiles.h"
#include "key.h"

static uint8_t addr = 0;
static uint8_t cursorx = 0, cursory = 0;
static bool atBottom = false;
static uint8_t lastkey = 0;
static bool capslock = false;
static bool alt = false;
static bool active = false;

/* plot a 4x8 bitmap */
static void lcdterm_plotchar(uint8_t x, uint8_t y, const uint8_t *pix)
{
    lcd_cs(1);
    lcd_setNormalReverse(1);
    lcd_setPos(x, y);
    lcd_txData(pix[0]);
    lcd_txData(pix[1]);
    lcd_txData(pix[2]);
    lcd_txData(pix[3]);
    lcd_cs(0);
}

/* plot cursor */
static void lcdterm_cursor(void)
{
    if (alt)
        lcdterm_plotchar(cursorx, cursory, tiles['*']);
    else
    if (capslock)
        lcdterm_plotchar(cursorx, cursory, tiles['-']);
    else
        lcdterm_plotchar(cursorx, cursory, tiles['_']);
}

void lcdterm_init(void)
{
    lcd_init();
    active = true;
    capslock = false;
    alt = false;
    addr = 0;
    cursorx = 0;
    cursory = 0;

    lcdterm_cursor();
}

/* reset lcd and don't use it again till lcdterm_init() called */
void lcdterm_stop(void)
{
    active = false;
    lcd_init();
}

/* apply alt key mapping */
static uint8_t alter(uint8_t k)
{
    switch(k)
    {
        case 'q': return '1';
        case 'w': return '2';
        case 'e': return '3';
        case 'r': return '@';
        case 't': return '#';
        case 'y': return '$';
        case 'u': return '&';
        case 'i': return '-';
        case 'o': return '+';
        case 'p': return '=';
        case 'a': return '4';
        case 's': return '5';
        case 'd': return '6';
        case 'f': return '\\';
        case 'g': return '_';
        case 'h': return ':';
        case 'j': return ';';
        case 'k': return '"';
        case 'l': return '\'';
        case 'z': return '7';
        case 'x': return '8';
        case 'c': return '9';
        case 'v': return '(';
        case 'b': return ')';
        case 'n': return '!';
        case 'm': return '.';
        case ',': return '?';
        case KSPK: return '0';
    }
    return k;
}

/* poll keyboard and update lcd */
void lcdterm_tick(void)
{
    uint8_t k;

    if (!active)
        return;

    k = key_get();
    if (k != lastkey)
    {
        lastkey = k;

        if (alt) /* apply alt */
            k = alter(k);
        else /* apply caps lock */
        if (capslock)
        {
            if(k >= 'a' && k <= 'z')
                k = (k-'a') + 'A';
        }

        /* handle special/mode keys */
        switch(k)
        {
            case KCAP:
            capslock = !capslock;
            lcdterm_cursor();
            return;

            case KALT:
            alt = !alt;
            lcdterm_cursor();
            return;

            case KSPK:
            case KPWR:
            case KMNU:
            case KONL:
            case KDWN:
            case KBYE:
            return;
        }

        /* feed console */
        if (0 != k)
        {
            if (console_rx_ready_callback())
                console_rx_callback(k);
        }
    }
}

void lcdterm_putc(uint8_t ch)
{
    if (!active)
        return;

    /* backspace */
    if (ch == '\b')
    {
        /* blank old cursor position */
        lcdterm_plotchar(cursorx, cursory, tiles[' ']);
        /* move left */
        cursorx -= 4;
        lcdterm_cursor();
        return;
    }

    /* bell */
    if (ch == '\a')
    {
        /* TODO */
        return;
    }

    /* font has 128 bitmaps */
    if (ch > 128)
        ch = 0;

    lcdterm_plotchar(cursorx, cursory, tiles[ch]);

    /* move cursor */
    cursorx += 4;
    if (cursorx >= 128)
        lcdterm_newline();

    lcdterm_cursor();
}

/* print a newline, scroll lcd */
void lcdterm_newline(void)
{
    uint8_t i;

    if (!active)
        return;

    /* blank old cursor */
    lcdterm_plotchar(cursorx, cursory, tiles[' ']);

    cursorx = 0;
    cursory++;

    if (cursory >= 8)
    {
        atBottom = true;
        cursory = 0;
    }
    if (atBottom)
    {
        /* clear the line */
        for (i=0;i<32;i++)
            lcdterm_plotchar(i<<2, cursory, tiles[' ']);

        addr += 8;
        lcd_cs(1);
        lcd_setAddr(addr);
        lcd_cs(0);
    }
}
