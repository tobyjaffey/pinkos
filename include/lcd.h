#ifndef LCD_H
#define LCD_H 1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define LCD_A0 P0_2
#define LCD_SSN P0_4
#define LCD_RST P1_1

void lcd_init(void);
void lcd_setPos(uint8_t row, uint8_t col);
void lcd_setAddr(uint8_t start);
void lcd_setNormalReverse(uint8_t normal);
void lcd_cls(uint8_t ch);
void lcd_txData(uint8_t ch);
void lcd_ssn(uint8_t on);
void lcd_cs(uint8_t on);

#endif

