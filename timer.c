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

static struct timer_entry *timers[MAX_TIMERS];

void timer__init()
{
    memset(timers,0,sizeof(timers));
}

int timer__add(uint32_t delta_ms, int (*callback)(void*), void *data)
{
    struct timer_entry *tmr;
    int i;

    tmr = (struct timer_entry*) malloc (sizeof(struct timer_entry));

    tmr->expires = HAL_GetTick() + (uint32_t)delta_ms;
    tmr->delta_ms = delta_ms;
    tmr->callback = callback;
    tmr->userdata = data;

    for (i=0;i<MAX_TIMERS;i++) {
        if (timers[i] == NULL) {
            timers[i] = tmr;
            break;
        }
    }

    if (i==MAX_TIMERS)
        i=-1;

    return i;
}

void timer__cancel(int timer)
{
    struct timer_entry *e = timers[timer];
    timers[timer] = NULL;
    if (e) {
        free(e);
    }
}
#if 0
void timer__reschedule(int timer, uint32_t newDelta)
{
    struct timer_entry *e = timers[timer];
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
            struct timer_entry *e = timers[i];

            if (e!=NULL) {

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
                        timers[i] =  NULL;
                        free( e );
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
