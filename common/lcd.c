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
#include "lcd.h"
#include "spi.h"
#include "clock.h"

/* send a data byte */
void lcd_txData(uint8_t ch)
{
    LCD_A0 = 1;
    spi_tx(ch);
}

/* send a control byte */
static void txCtl(uint8_t ch)
{
    LCD_A0 = 0;
    spi_tx(ch);
}

/* set cursor position */
void lcd_setPos(uint8_t x, uint8_t y)
{
    txCtl(0xb0 + y);
    txCtl(0x00 + (x & 0x0f));
    txCtl(0x10 + ( (x>>4) & 0x0f));
}

/* set vertical offset into display RAM */
void lcd_setAddr(uint8_t start)
{
    txCtl(0x40 | (start & 0x3f));
}

void lcd_setNormalReverse(uint8_t normal)
{ 
    txCtl(0xa6 | (normal & 0x01) );
}

/* clear every pixel - even on the edge */
void lcd_cls(uint8_t ch)
{
    uint8_t y, x;
    for (y=0;y<9;y++)
    {
        lcd_setPos(0, y);
        for (x=0;x<132;x++)
            lcd_txData(ch);
    }
}

void lcd_init(void)
{
    /* SSN and A0 as outputs */
    P0DIR |= BIT4 | BIT2;
    /* reset as output */
    P1DIR |= BIT1;

    /* reset lcd */
    LCD_RST = 0;
    clock_delayms(1);
    LCD_RST = 1;

    lcd_cs(1);
    txCtl(0xe2);
    txCtl(0x24);
    txCtl(0x81);
    txCtl(0x60);
    txCtl(0xe6);
    txCtl(0x00);
    txCtl(0x2f);
    txCtl(0xa1);
    txCtl(0xaf);
    txCtl(0xa4);
    lcd_setNormalReverse(0);
    lcd_setAddr(0);
    lcd_cls(0xFF);
    lcd_cs(0);
}

void lcd_cs(uint8_t on)
{
    LCD_SSN = !on;
}



