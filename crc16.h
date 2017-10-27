#ifndef __CRC16_H__
#define __CRC16_H__

#include <inttypes.h>
#include <sys/types.h>

typedef uint16_t crc_t;

static inline void crc16__reset(crc_t *crc)
{
    *crc = 0xffff;
}

void crc16__append(crc_t *crc, uint8_t data);
void crc16__appendBuffer(crc_t *crc, const uint8_t *data, size_t size);

#endif
