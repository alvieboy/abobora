#ifndef __DISTANCE_H__
#define __DISTANCE_H__

// Use TIM3 (TIM4 is not available) unless impossible. check defs.h

void distance_init();
int distance_read();

void distance_ping();

#endif
