#include "common.h"

#pragma sdcc_hash +
#define ISR_BODY(x,handler) \
    push psw \
    jnb  psw.1, 00001$ \
 \
    pop  psw \
    lcall handler \
    reti \
 \
00001$: \
    pop  psw \
    ljmp #APPLICATION_OFFSET + (x*8+3)


#define DEF_ISR(x,handler) \
void x##_isr(void) __interrupt x \
{ \
_asm \
    ISR_BODY(x,handler) \
_endasm; \
}

/* declare interrupt vectors and generate code to call into pinkos or app */

#ifdef BOARD_IMME_DONGLE
DEF_ISR(URX0_VECTOR, _uart0_isr)
#endif

DEF_ISR(RFTXRX_VECTOR, _rftxrx_isr)
DEF_ISR(RF_VECTOR, _rf_isr)


