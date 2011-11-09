/*
 * This file is part of PinkOS
 * Copyright (c) 2010, Joby Taffey <jrt@hodgepig.org>
 * Copyright (c) 2010, Travis Goodspeed <travis@radiantmachines.com>
 * Copyright (c) 2010, http://daveshacks.blogspot.com
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
#include "key.h"
#include "clock.h"

// #define DEBUG 1

#ifdef DEBUG
#include "console.h"
#endif

#define KEY(row,col) keychars[row*10+col]

//8 rows, 10 columns
static const uint8_t keychars[]=
{
    //gnd 0_1   1_2   1_3   1_4   1_5   1_6   1_7   0_6   0_7
    //row 0, gnd
    0x00, 0x00, 'o',  'k',  'n',  'm',  KPWR, 'p',  0x00, 0x00,
    //row 1
    0x00, 0x00, 'y',  'g',  'c',  ' ',  '<',  ',',  KMNU, '>',
    //row 2
    0x00, 0x00, 0x00, 'q',  'w',  'e',  'r',  't',  'u',  'i',
    //row 3
    0x00, 0x00, 0x00, 0x00, 'a',  's',  'd',  'f',  'h',  'j',
    //row 4
    0x00, 0x00, 0x00, 0x00, 0x00, KCAP, 'z',  'x',  'v', 'b',
    //row 5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, KSPK, KALT, KONL, '\b',
    //row 6
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, KBYE, '^',  KDWN,
    //row 7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\r', 'l'
};

void key_init(void)
{
}

static uint8_t realkeyscan(void)
{
    uint8_t row, col;

    // All input
    P0DIR &= ~(BIT1 | BIT6 | BIT7);
    P1DIR &= ~(BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

    P0 |= BIT1 | BIT6 | BIT7;
    P1 |= BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    // wait a bit, FIXME replace with timer tick and state machine
    clock_delayms(1);

    for(row=0;row<8;row++)
    {
        col = row; // nothing
        switch(row)
        { 
            case 0: // ground
            default:
            break;
            case 1: // P0_1
            P0DIR |= BIT1;
            P0 &= ~BIT1;
            break;
            case 2: // P1_2
            P1DIR |= BIT2;
            P1 &= ~BIT2;
            break;
            case 3: // P1_3
            P1DIR |= BIT3;
            P1 &= ~BIT3;
            break;
            case 4: // P1_4
            P1DIR |= BIT4;
            P1 &= ~BIT4;
            break;
            case 5: // P1_5
            P1DIR |= BIT5;
            P1 &= ~BIT5;
            break;
            case 6: // P1_6
            P1DIR |= BIT6;
            P1 &= ~BIT6;
            break;
            case 7: // P1_7
            P1DIR |= BIT7;
            P1 &= ~BIT7;
            break;
        }

        if(~P0 & BIT1)
            col = 1;
        if(~P1 & BIT2)
            col = 2;
        if(~P1 & BIT3)
            col = 3;
        if(~P1 & BIT4)
            col = 4;
        if(~P1 & BIT5)
            col = 5;
        if(~P1 & BIT6)
            col = 6;
        if(~P1 & BIT7)
            col = 7;
        if(~P0 & BIT6)
            col = 8;
        if(~P0 & BIT7)
            col = 9;

        if(col != row)
        {
#ifdef DEBUG
            console_puthex8(row);
            console_puthex8(col);
#endif
            return KEY(row,col);
        }
    }

    return 0x00;
}

uint8_t key_get(void)
{
    uint8_t key = realkeyscan();

    // Debounce
    while(key!=realkeyscan())
        key=realkeyscan();

    // All input
    P0DIR &= ~(BIT1 | BIT6 | BIT7);
    P1DIR &= ~(BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P0 |= BIT1 | BIT6 | BIT7;
    P1 |= BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    return key;
}

