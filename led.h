#ifndef __LED_H__

#include "defs.h"

typedef uint8_t indexed_pixel_t;
typedef uint16_t pixel_t; // RGB565

void led_init();
void led_txchunk();
void led_flush();
void led_setpallete(pixel_t*);
void led_setpixel(int index, uint8_t value);


#endif
