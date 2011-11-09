#ifndef UART0_H
#define UART0_H 1

void uart0_init(void);
void uart0_putc(uint8_t ch);

extern void uart0rx_handler(void);

#endif

