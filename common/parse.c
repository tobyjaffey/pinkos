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
#include "parse.h"

/* hex nibble to int */
static char digit_to_int(char ch)
{
    uint8_t r;
    if (ch >= 'a')
        r = 'a' - 10;
    else
    {
        if (ch >= 'A')
            r = 'A' - 10;
        else
        {
            if (ch <= '9')
                r = '0';
            else
                r = 0;
        }
    }

    return ch - r;
}

/* decode pairs of hex digits "DEADBEEFDECAFBAD" */
uint8_t parse_hexdata(const char data *str, uint8_t data *outbuf, uint8_t outbuf_len)
{
    uint8_t len = 0;
    uint8_t i;
    const char data *p = str;

    /* strlen */
    while(*p++)
        len++;

    /* sanity check length */
    if (len & 1 || ((len>>1) > outbuf_len))
        return 0;

    for (i=0;i<len;i+=2)
        outbuf[i>>1] = (digit_to_int(str[i]) << 4) | digit_to_int(str[i+1]);

    return len >> 1;
}

/* parse binary, hex and decimal numbers, b101 hF8 0xF8 123 */
uint16_t parse_number(const char data *str)
{
    int8_t base = 10;
    uint8_t i;
    char c;
    char digit;
    uint8_t len = 0;
    const char data *p = str;
    uint16_t result = 0;

    // strlen
    while(*p++)
        len++;

    result = 0;

    for (i=0;i<len;i++)
    {
        c = str[i];

        if (result == 0)
        {
            if ((i == 0 || i == 1) && c == 'b')     /* 0b/b for binary */
            {
                if (len < 2)
                    return 0;
                base = 2;
                continue;
            }
            else
            if (i==0 && c == 'h')       /* h for hex */
            {
                if (len < 2)
                    return false;
                base = 16;
                continue;
            }
            else
            if (i == 1 && c == 'x')     /* 0x for hex */
            {
                if (len < 3)
                    return false;
                base = 16;
                continue;
            }
        }

        digit = digit_to_int(c);

        if (digit < base)
            result = result * base + digit;
        else
            return 0;
    }
    return result;
}


