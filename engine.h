#ifndef __ENGINE_H__
#define __ENGINE_H__


typedef enum {
    ENGINE_IDLE,
    ENGINE_APPROACH,
    ENGINE_CLOSEUP,
    ENGINE_BLOWUP
} main_state_t;

void engine_init();
main_state_t engine_get_state();
void engine_loop();


#endif
