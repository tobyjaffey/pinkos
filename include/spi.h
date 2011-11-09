#ifndef SPI_H
#define SPI_H 1

// Clock rate of approx. 2.5 Mbps for 26 MHz Xtal clock
#define SPI_BAUD_M  170
#define SPI_BAUD_E  16

void spi_init(void);
void spi_tx(uint8_t ch);

#endif

