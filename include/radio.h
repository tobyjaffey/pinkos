#ifndef RADIO_H
#define RADIO_H 1

void radio_init(uint8_t device_address);
bool radio_tx(void);
void radio_rx(void);
void radio_tick(void);
bool radio_tx_ready(void);

data volatile const uint8_t *radio_getbuf(void);

#endif

