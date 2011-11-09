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
#include "clock.h"
#include "packet.h"

// #define DEBUG 1

#define PKTBUF_MAX 32

/* shared tx and rx packet buffer */
static data volatile uint8_t pktbuf[PKTBUF_MAX];
static volatile uint8_t pktbuf_index = 0;

/* flags */
static volatile bool errflag = false;
static volatile bool receiving = false;
static volatile bool pkt_complete = false;

/* external access to buffer */
data volatile const uint8_t *radio_getbuf(void)
{
    return pktbuf;
}

/* wait for MARCSTATE change */
static bool wait_rfstate(uint8_t state)
{
    uint8_t count = 0xFF;
    while(MARCSTATE != state && count != 0)
    {
        clock_delayms(1);
        count--;
    }

    if (count == 0)
    {
#ifdef DEBUG
        console_puts("wait_rfstate: ");
        console_puthex8(state);
        console_newline();
#endif
        return false;
    }
    return true;
}

void radio_init(uint8_t device_address)
{
    /* 250kbaud @ 868MHz from RF Studio 7 */
    SYNC1      =     0xD3;       // Sync Word, High Byte 
    SYNC0      =     0x91;       // Sync Word, Low Byte 
    PKTLEN     =     0xFF;       // Packet Length 
    PKTCTRL1   =     0x04;       // Packet Automation Control 
    PKTCTRL0   =     0x05;       // Packet Automation Control 
    ADDR       =     0x00;       // Device Address 
    CHANNR     =     0x00;       // Channel Number 
    FSCTRL1    =     0x0C;       // Frequency Synthesizer Control 
    FSCTRL0    =     0x00;       // Frequency Synthesizer Control 
    FREQ2      =     0x21;       // Frequency Control Word, High Byte 
    FREQ1      =     0x65;       // Frequency Control Word, Middle Byte 
    FREQ0      =     0x6A;       // Frequency Control Word, Low Byte 
    MDMCFG4    =     0x2D;       // Modem configuration 
    MDMCFG3    =     0x3B;       // Modem Configuration 
    MDMCFG2    =     0x13;       // Modem Configuration 
    MDMCFG1    =     0x22;       // Modem Configuration 
    MDMCFG0    =     0xF8;       // Modem Configuration 
    DEVIATN    =     0x62;       // Modem Deviation Setting 
    MCSM2      =     0x07;       // Main Radio Control State Machine Configuration 
    MCSM1      =     0x30;       // Main Radio Control State Machine Configuration 
    MCSM0      =     0x18;       // Main Radio Control State Machine Configuration 
    FOCCFG     =     0x1D;       // Frequency Offset Compensation Configuration 
    BSCFG      =     0x1C;       // Bit Synchronization Configuration 
    AGCCTRL2   =     0xC7;       // AGC Control 
    AGCCTRL1   =     0x00;       // AGC Control 
    AGCCTRL0   =     0xB0;       // AGC Control 
    FREND1     =     0xB6;       // Front End RX Configuration 
    FREND0     =     0x10;       // Front End TX Configuration 
    FSCAL3     =     0xEA;       // Frequency Synthesizer Calibration 
    FSCAL2     =     0x2A;       // Frequency Synthesizer Calibration 
    FSCAL1     =     0x00;       // Frequency Synthesizer Calibration 
    FSCAL0     =     0x1F;       // Frequency Synthesizer Calibration 
    TEST2      =     0x88;       // Various Test Settings 
    TEST1      =     0x31;       // Various Test Settings 
    TEST0      =     0x09;       // Various Test Settings 
    PA_TABLE7  =     0x00;       // PA Power Setting 7 
    PA_TABLE6  =     0x00;       // PA Power Setting 6 
    PA_TABLE5  =     0x00;       // PA Power Setting 5 
    PA_TABLE4  =     0x00;       // PA Power Setting 4 
    PA_TABLE3  =     0x00;       // PA Power Setting 3 
    PA_TABLE2  =     0x00;       // PA Power Setting 2 
    PA_TABLE1  =     0x00;       // PA Power Setting 1 
    PA_TABLE0  =     0x50;       // PA Power Setting 0 
    IOCFG2     =     0x00;       // Radio Test Signal Configuration (P1_7) 
    IOCFG1     =     0x00;       // Radio Test Signal Configuration (P1_6) 
    IOCFG0     =     0x00;       // Radio Test Signal Configuration (P1_5) 
    PARTNUM    =     0x01;       // Chip ID[15:8] 
    VERSION    =     0x03;       // Chip ID[7:0] 
    FREQEST    =     0x00;       // Frequency Offset Estimate from Demodulator 
    LQI        =     0x00;       // Demodulator Estimate for Link Quality 
    RSSI       =     0x80;       // Received Signal Strength Indication 
    MARCSTATE  =     0x01;       // Main Radio Control State Machine State 
    PKTSTATUS  =     0x00;       // Packet Status 
    VCO_VC_DAC =     0x94;       // Current Setting from PLL Calibration Module 

    /****************************************************/

    /* turn power up to 10dBm */
    PA_TABLE0  = 0xC2;

    /* enable hardware address filtering */
    PKTCTRL1 |= 0x02;

    /* set our address */
    ADDR = device_address;

    /* enable interrupts */
    RFIF = 0;
    IEN2 |= IEN2_RFIE;
    RFTXRXIE = 1;
    RFIM |= RFIF_IRQ_DONE   |
            RFIF_IRQ_TXUNF  |
            RFIF_IRQ_RXOVF  |
            RFIF_IRQ_SFD    |
            RFIF_IRQ_TIMEOUT;

    /* enter idle */
    RFST = RFST_SIDLE;
    wait_rfstate(MARC_STATE_IDLE);

    /* start receiving */
    radio_rx();
}

void rftxrx_isr(void)
#ifndef PINKOS
__interrupt RFTXRX_VECTOR
#endif
{
    if (MARCSTATE == MARC_STATE_TX)
    {
        /* send next byte */
        if (pktbuf_index < pktbuf[0]+1)
            RFD = pktbuf[pktbuf_index++];
        else
            errflag = true;
    }
    else
    if (MARCSTATE == MARC_STATE_RX)
    {
        /* fetch next byte */
        if (pktbuf_index == 0 || pktbuf_index < pktbuf[0]+1 && pktbuf_index < PKTBUF_MAX)
            pktbuf[pktbuf_index++] = RFD;
        else
        {
            if (pktbuf_index == pktbuf[0]+1)
                pkt_complete = true;
            else
                errflag = true;
        }
    }
}

void rf_isr(void)
#ifndef PINKOS
__interrupt RF_VECTOR
#endif
{
    /* clear flags */
    S1CON &= ~(S1CON_RFIF_1 + S1CON_RFIF_0);

    if (RFIF & RFIF_IRQ_DONE)
    {
        if (pktbuf_index == pktbuf[0]+1)
        {
            /* finished tx */
            RFST = RFST_SIDLE;
            pktbuf_index = 0;
            pkt_complete = true;
        }
        else
        {
            /* failed tx */
            RFST = RFST_SIDLE;
            pktbuf_index = 0;
            errflag = true;
        }
    }

    /* errors */
    if ((RFIF & RFIF_IRQ_TXUNF) || (RFIF & RFIF_IRQ_RXOVF) || (RFIF & RFIF_IRQ_TIMEOUT))
    {
        RFST = RFST_SIDLE;
        pktbuf_index = 0;
        errflag = true;
    }

    /* sync */
    if (RFIF & RFIF_IRQ_SFD)
    {
        pktbuf_index = 0;
    }

    /* clear interrupt */
    RFIF = 0;
}

void radio_tick(void)
{
    if (receiving)
    {
        if (errflag)
        {
            /* start again */
            radio_rx();
            return;
        }
        if (pkt_complete)
        {
            /* check CRC */
            if (LQI & 0x80)
            {
                packet_process();
            }
            else
            {
                console_puts("RX Bad CRC");
                console_newline();
            }

            /* listen for more */
            radio_rx();
        }
    }
    else
    {
        /* we're transmitting */
        if (errflag)
        {
            console_puts("TX ERR");
            console_newline();
            radio_rx();
        }
        if (pkt_complete)
        {
            /* finished sending, start receiving */
            radio_rx();
        }
    }
}

/* transmit contents of buffer */
bool radio_tx(void)
{
    /* idle */
    RFST = RFST_SIDLE;
    wait_rfstate(MARC_STATE_IDLE);

    /* init flags */
    pktbuf_index = 0;
    receiving = false;
    pkt_complete = false;
    errflag = false;

    /* transmit */
    RFST = RFST_STX;
    wait_rfstate(MARC_STATE_TX);

    /* block until sent or failed */
    while(!pkt_complete && !errflag)
    {
#ifdef DEBUG
        console_putc('.');
#endif
    }
#ifdef DEBUG
    console_newline();
#endif

    return !errflag;
}

/* receive packet to buffer */
void radio_rx(void)
{
    /* idle */
    RFST = RFST_SIDLE;
    wait_rfstate(MARC_STATE_IDLE);

    /* init flags */
    errflag = false;
    pktbuf_index = 0;
    receiving = true;
    pkt_complete = false;

    /* receive */
    RFST = RFST_SRX;
    wait_rfstate(MARC_STATE_RX);
}


