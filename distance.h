#ifndef __DISTANCE_H__
#define __DISTANCE_H__

#include <inttypes.h>
// Use TIM3 (TIM4 is not available) unless impossible. check defs.h

void distance_init();
int distance_read(uint32_t *value, uint8_t *closeness);

void distance_ping();


#endif
