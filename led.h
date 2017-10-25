#ifndef __LED_H__

#include "defs.h"

typedef uint8_t indexed_pixel_t;
typedef uint32_t pixel_t;

void led_init();
void led_txchunk();
void led_flush();
void led_setpallete(pixel_t*);

#endif
