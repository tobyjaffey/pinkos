#ifndef LCDTERM_H
#define LCDTERM_H 1

void lcdterm_init(void);
void lcdterm_stop(void);
void lcdterm_tick(void);
void lcdterm_putc(uint8_t ch);
void lcdterm_newline(void);

#endif

