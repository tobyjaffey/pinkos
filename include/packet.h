#ifndef PACKET_H
#define PACKET_H 1

#define BROADCAST_ADDR 0x00

#define PACKET_SEQ_MASK (~0x80)
#define PACKET_SEQ_REQ_ACK 0x80

enum
{
    PACKET_TYPE_ACK = 0x00,
    PACKET_TYPE_PUTS,
    PACKET_TYPE_LED,
    PACKET_TYPE_GFXINIT,
    PACKET_TYPE_GFXPLOTLINE,
};

typedef struct
{
    uint8_t len;
    uint8_t dstAddr;
    uint8_t srcAddr;
    uint8_t seq;
    uint8_t type;
} packet_t;

void packet_process(void);

data uint8_t *packet_build(uint8_t type, uint8_t len);
void packet_send(uint8_t dstAddr, uint8_t seq);

extern void packet_rx_callback(uint8_t srcAddr, uint8_t seq, uint8_t type, const data uint8_t *buf, uint8_t len);

#endif

