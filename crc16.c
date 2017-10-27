#include "crc16.h"

void crc16__append(crc_t *crc, uint8_t data)
{
    data ^= *crc&0xff;
    data ^= data << 4;
    *crc = ((((uint16_t)data << 8) | ((*crc>>8)&0xff)) ^ (uint8_t)(data >> 4)
            ^ ((uint16_t)data << 3));
}

void crc16__appendBuffer(crc_t *crc, const uint8_t *data, size_t size)
{
    while (size--) {
        crc16__append(crc, *data);
        data++;
    }
}
