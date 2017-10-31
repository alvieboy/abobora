#include "engine.h"
#include "distance.h"
#include "uart.h"

#define DISTANCE_NONE 3
#define DISTANCE_FAR  2
#define DISTANCE_CLOSEUP 1
#define DISTANCE_BLOW 0

static main_state_t main_state;

main_state_t engine_get_state()
{
    return main_state;
}


void engine_init()
{
    main_state = ENGINE_IDLE;
}

void engine_idle()
{
    if (get_distance() != DISTANCE_NONE) {
        outstring("Approach\r\n");
        effect();
        main_state = ENGINE_APPROACH;
    }
}

void engine_approach()
{
    int d = get_distance();
    if (d == DISTANCE_NONE) {
        outstring("Idling\r\n");
        main_state = ENGINE_IDLE;
    } else if (d != DISTANCE_FAR) {
        main_state = ENGINE_CLOSEUP;
    }
}

void engine_closeup()
{
    int d = get_distance();
    if ((d == DISTANCE_NONE) || (d==DISTANCE_FAR)) {
        main_state = ENGINE_IDLE;
    } else if (d != DISTANCE_CLOSEUP) {
        main_state = ENGINE_BLOWUP;
    }
}

void engine_blowup()
{
    int d = get_distance();
    // In blowup, we have sub-stages.

}

void engine_loop()
{
    switch(main_state)
    {
    case ENGINE_IDLE:
        engine_idle();
        break;
    case ENGINE_APPROACH:
        engine_approach();
        break;
    case ENGINE_CLOSEUP:
        engine_closeup();
        break;
    case ENGINE_BLOWUP:
        engine_blowup();
        break;
    }
}
