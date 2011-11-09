#ifndef CONSOLE_H
#define CONSOLE_H 1

void console_init(void);
void console_tick(void);

void console_prompt(void);

void console_set_linemode(bool linemode);

void console_putc(uint8_t c);
bool console_getc(uint8_t *c);
bool console_key_poll(uint8_t *c);

void console_newline(void);
void console_puts(uint8_t *str);
void console_puthex8(uint8_t h);
void console_puthex16(uint16_t h);
void console_puthex32(uint32_t h);
void console_putdec(uint32_t i);
void console_putbin(uint8_t b);
void console_putsmem(const uint8_t *a, const uint8_t *b);

void console_rx_callback(uint8_t c);
bool console_rx_ready_callback(void);

#endif

