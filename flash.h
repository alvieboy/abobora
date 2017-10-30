#ifndef __FLASH_H__
#define __FLASH_H__

#include <inttypes.h>

typedef struct {
    uint8_t *cached_sector;
    uint8_t cached_dirty:1;
    int16_t cached_sector_number:15;
} flash_control_t;

void flash_control_init(flash_control_t *ctrl);
void flash_control_release(flash_control_t *ctrl);

int spiflash_read_cached(flash_control_t *ctrl, unsigned address, uint8_t *target, unsigned size);
int spiflash_write_cached(flash_control_t *ctrl, unsigned address, const uint8_t *data, unsigned size);
int spiflash_flush(flash_control_t *ctrl);


#endif
