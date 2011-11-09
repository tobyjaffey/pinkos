#ifndef PARSE_H
#define PARSE_H 1

uint16_t parse_number(const char data *str);
uint8_t parse_hexdata(const char data *str, uint8_t data *outbuf, uint8_t outbuf_len);

#endif

