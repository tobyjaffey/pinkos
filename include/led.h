#ifndef LED_H
#define LED_H 1

#ifdef BOARD_IMME_DONGLE
#define led_init()      P0DIR |= BIT6
#define led_on()        P0 &= ~BIT6
#define led_off()       P0 |= BIT6
#define led_toggle()    P0 ^= BIT6
#endif

#ifdef BOARD_IMME_HANDSET
#define led_init()      P2DIR |= BIT3
#define led_on()        P2 &= ~BIT3
#define led_off()       P2 |= BIT3
#define led_toggle()    P2 ^= BIT3
#endif

#endif

