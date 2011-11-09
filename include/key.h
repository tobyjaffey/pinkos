#ifndef KEY_H
#define KEY_H 1

#define KPWR 0x01
#define KMNU 0x03
#define KCAP 0x82
#define KSPK 0x83
#define KALT 0x84
#define KONL 0x85
#define KDWN 0x87
#define KBYE 0x02

void key_init(void);
uint8_t key_get(void);

#endif

