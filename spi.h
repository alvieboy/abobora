#ifndef __SPI_H__
#define __SPI_H__

#include <inttypes.h>

void spi_init();
void spi_enable();
void spi_disable();
void spi_write(uint8_t);
void spi_write32(uint32_t);
uint8_t spi_read();

#endif
