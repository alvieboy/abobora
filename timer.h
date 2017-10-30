#ifndef __TIMER_H__
#define __TIMER_H__


#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup Timers Timers management routines
 *  @{
 */

/**
 * @brief Initialize the timers subsystem
 *
 * This function shall be called before any other timers function
 */

void timer__init();

/**
 * @brief Add a timer
 *
 * The callback shall return "0" for periodic calling, or a different
 * value to cancel the timer.
 *
 * @param delta_ms Interval, in milliseconds,  at which callback shall be called.
 * @param callback The callback to be called
 * @param data The argument to be passed to the callback function
 *
 * @return The timer id on success, a negative number otherwise
 */
int timer__add(uint32_t delta_ms, int (*callback)(void*), void *data);

/**
 * @brief Cancel a previously created timer
 * @param timer The timer id
 */
void timer__cancel(int timer);


void timer__iterate();

/** @}; */

#ifdef __cplusplus
}
#endif

#endif
