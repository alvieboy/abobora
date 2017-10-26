#include "distance.h"
#include "defs.h"
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal.h>

void distance_init()
{
    GPIO_InitTypeDef init;

    DISTANCE_TRIG_GPIO_CLK_ENABLE();

    init.Pin = DISTANCE_TRIG_GPIO_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init( DISTANCE_TRIG_GPIO, &init );
}

static void distance_delay()
{
    volatile int counter = 80;
    while (counter--) {
    }
}

int distance_read()
{
    // TBD

    HAL_GPIO_WritePin( DISTANCE_TRIG_GPIO, DISTANCE_TRIG_GPIO_PIN, 1);
    distance_delay();
    HAL_GPIO_WritePin( DISTANCE_TRIG_GPIO, DISTANCE_TRIG_GPIO_PIN, 0);



    return -1;
}
