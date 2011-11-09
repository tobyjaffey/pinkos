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
#include "uart0.h"
#include "console.h"

void uart0_isr(void)
#ifndef PINKOS
__interrupt URX0_VECTOR
#endif
{
    URX0IF = 0;

    if (console_rx_ready_callback())
        console_rx_callback(U0DBUF);
}

void uart0_init(void)
{
    PERCFG = (PERCFG & ~PERCFG_U0CFG) | PERCFG_U1CFG;
    P0SEL |= BIT5 | BIT4 | BIT3 | BIT2;

    U0CSR = 0x80 | 0x40;    // UART, RX on

    U0BAUD = 34;    // 115200
    U0GCR = 12;

    URX0IF = 1;
    URX0IE = 1;
}

void uart0_putc(uint8_t ch)
{
    U0DBUF = ch;
    while(!(U0CSR & U0CSR_TX_BYTE)); // wait for byte to be transmitted
    U0CSR &= ~U0CSR_TX_BYTE;         // Clear transmit byte status
}

