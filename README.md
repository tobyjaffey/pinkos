PinkOS
======

PinkOS is a minimal operating system/bootloader for the GirlTech IM-Me and CC111x.
http://code.google.com/p/pinkos/
Joby Taffey <jrt@hodgepig.org>

It provides a bootloader with a command line console (either serially or using
LCD + keyboard) and both peer-to-peer and broadcast packet radio.

To get started, read precompiled/README

Status:
-------

PinkOS is working, but incomplete.

Desired features include:
 * flash write support in the bootloader
 * a reliable transport protocol
 * flow controlled serial
 * flash based non-volatile config data
 * support for USB CDC on CC1111

Building:
---------

Currently, radio network addresses are hardcoded into each firmware.
Set the address at compile time with DEVADDR=<0-255>. Note - 0 is the broadcast address.

To build and install PinkOS:

$ make erase

For IM-Me handset:
$ make clean && make DEVICE=HANDSET DEVADDR=<address> install

For IM-Me dongle:
$ make clean && make DEVICE=DONGLE DEVADDR=<address> install

To build and install an application:
$ cd apps/speccan && make install     (note: handset only)

or

$ cd apps/shellapp && make clean && make DEVICE=DONGLE install
$ cd apps/shellapp && make clean && make DEVICE=HANDSET install


LCD Console:
------------

The IM-Me handset version provides a text console using the LCD and keyboard:
 * caps lock is caps lock not shift
 * alt behaves like caps lock
 * the cursor will change to signal the mode
 * key repeat is not supported

Serial Console:
---------------

The IM-Me dongle version provides the console over TTL level RS232 at 115200 8-n-1 with no flow control.
See http://code.google.com/p/pinkos/wiki/IMMeDongleSerial for the required hardware modifications.

Radio Packet Format:
--------------------

All radio packets begin with a 5 byte header:
uint8_t len;        // packet length (not including this byte)
uint8_t dstAddr;    // destination address
uint8_t srcAddr;    // source address
uint8_t seq;        // sequence number, set top bit to request an ACK
uint8_t type;       // packet type

Console Commands:
-----------------

help
    show help text

reset
    reset

boot
    boot application from 0x4000

xdata <addr16> [data8]
    peek/poke xram
    "xdata 0xDF2E 0x50" set PA_TABLE0 to 0x50
    "xdata 0xDF2E 0x50" set PA_TABLE0 to 0xC2 (10dBm)
    "xdata 0xDF36" read PARTNUM (chip id)

pdata <addr16> [data8]
    peek/poke ram

tx <dst8> <seq8> <typ8> <str>
    build and send a packet to dst with sequence number seq with type typ containing payload str
    (str must not contain spaces)
    "tx 1 0x80 1 hello" sends "hello" to address 1 with a packet type of PACKET_TYPE_PUTS with the ACK request bit set
    "tx 0 0 1 hello" sends "hello" to the broadcast address

led <bool>
    control local led
    0 = off, 1 = on, other = toggle

teleled <dst8> <bool>
    control a remote led
    "teleled 0 1" broadcasts a "turn on" packet
    "teleled 1 0" turns off the led of node 1

gfxinit <dst8>
    enable graphics mode on a remote device
    suspends text console allowing gfxplots

gfxplot <dst8> <x> <y> HHHHHHHHHHHHHHHH
    plot an 8x8 block on a remote device (0 = black, 1 = white)
    0 <= x < 132, 0 <= y < 8
    "gfxplot 0 0 0 F0F0F0F00F0F0F0F" broadcast an 8x8 checkerboard to the top left

gfxtest <dst8>
    send test pattern to remote device

Authors
=======

Joby Taffey <jrt@hodgepig.org>

Portions Copyright (c) 2010:
    Travis Goodspeed <travis@radiantmachines.com>
    http://daveshacks.blogspot.com



