#include <malloc.h>
#include "timer.h"
#include <string.h>

/* Synchronous timers */

#define TIMER_ONESHOT 1
#define TIMER_CONTINUOUS 0

#define MAX_TIMERS 16

struct timer_entry
{
    uint32_t expires;
    uint32_t delta_ms;
    int (*callback)(void*userdata);
    void *userdata;
};

struct timer_entry localtimerlist[MAX_TIMERS];

void timer__init()
{
    for (int i=0;i<MAX_TIMERS;i++) {
        localtimerlist[i].expires = 0;
    }
}

int timer__add(uint32_t delta_ms, int (*callback)(void*), void *data)
{
    int i;
    for (i=0;i<MAX_TIMERS;i++) {
        if (localtimerlist[i].expires == 0) {
            localtimerlist[i].expires = HAL_GetTick() + (uint32_t)delta_ms;
            localtimerlist[i].delta_ms = delta_ms;
            localtimerlist[i].callback = callback;
            localtimerlist[i].userdata = data;
            return i;
        }
    }
    return -1;
}

void timer__cancel(int timer)
{
    localtimerlist[timer].expires = 0;
}
#if 0
void timer__reschedule(int timer, uint32_t newDelta)
{
    struct timer_entry *e = localtimerlist[timer];
}
#endif

void timer__iterate()
{
    int i;
    int retry;

    do {
        retry = 0;
        uint32_t now = HAL_GetTick();
        for (i=0;i<MAX_TIMERS;i++) {
            struct timer_entry *e = &localtimerlist[i];

            if (e->expires != 0) {

                if (e->expires <= now) {
#if 0
                    UARTPrint("Timer ");
                    printhexbyte(i);
                    UARTPrint(" expires ");
                    printhex((unsigned)e->expires);
                    UARTPrint(" now ");
                    printhex((unsigned)now);
                    UARTPrint("\r\n");
#endif
                    if (e->callback( e->userdata )!=0) {
                        localtimerlist[i].expires = 0;
                        retry = 1;
                        break; // Stop processing.
                    } else {
                        e->expires += e->delta_ms;
                    }
                }
            }
        }
    } while (retry);
}
