#include "distance.h"
#include "defs.h"
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal.h>

#define DISTANCE_THRESHOLD 50000 // discard everything above 50ms this gives ~15meters
#define DIST_ARRAY_SIZE 5
#define METERS_PER_TICK 0.000314f

static TIM_HandleTypeDef    TimHandle;
static TIM_IC_InitTypeDef ICHandle;



uint32_t distance[DIST_ARRAY_SIZE] = {0};



void distance_init()
{
    /*Trigger GPIO init*/

    GPIO_InitTypeDef init;

    DISTANCE_TRIG_GPIO_CLK_ENABLE();

    init.Pin = DISTANCE_TRIG_GPIO_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init( DISTANCE_TRIG_GPIO, &init );
    
    GPIO_InitTypeDef GPIOinit;

    GPIOinit.Pin = DISTANCE_ECHO_GPIO_PIN;
    GPIOinit.Mode = GPIO_MODE_AF_INPUT;
    GPIOinit.Pull = GPIO_PULLDOWN;
    GPIOinit.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init( DISTANCE_ECHO_GPIO, &GPIOinit );

    TimHandle.Instance = DISTANCE_TIM;
    TimHandle.Init.Prescaler         = 36;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;
    
    
    // Timer 3 channel 1 input capture configuration
    ICHandle.ICPolarity =  TIM_ICPOLARITY_FALLING;     //Capture a rising edge
    ICHandle.ICSelection = TIM_ICSELECTION_DIRECTTI;   // => IC1
    ICHandle.ICPrescaler = 0 ;
    ICHandle.ICFilter = 0;                          //If there is noise during the beging of capture specify number of rising edges to ignore max of 0xF

    //HAL_TIM_ConfigTI1Input(TIM_HandleTypeDef * htim, uint32_t TI1_Selection)

    HAL_TIM_IC_Init(&TimHandle);
    HAL_TIM_IC_ConfigChannel(&TimHandle, &ICHandle,TIM_CHANNEL_1);  // => PA6

    return;
}






static void distance_delay()
{
    volatile int counter = 40;
    while (counter--) {
    }
}

void distance_ping()
{
    //Send trigger pulse
    //Start timer in input capture mode TBD
    //HAL_TIM_IC_Start(&TimHandle, TIM_CHANNEL_1);

    HAL_GPIO_WritePin( DISTANCE_TRIG_GPIO, DISTANCE_TRIG_GPIO_PIN, 1);

    //Start timer in input capture mode
    HAL_TIM_IC_Start(&TimHandle, TIM_CHANNEL_1);
    
    distance_delay();
    HAL_GPIO_WritePin( DISTANCE_TRIG_GPIO, DISTANCE_TRIG_GPIO_PIN, 0);
}

int distance_read(uint32_t *value)
{
    uint32_t dist = 0;
    
    for(int i = 0 ;i < DIST_ARRAY_SIZE; i++)
        dist += distance[i];
        
    *value = (uint32_t)(((float)dist/(float)DIST_ARRAY_SIZE)*METERS_PER_TICK);
        
    return 0;
}

uint8_t distance_meas_idx = 0;

void distance_echo_interrupt()
{
    //Stop timer in input capture mode TIM3
    HAL_TIM_IC_Stop(&TimHandle, TIM_CHANNEL_1);
    
    if(TIM3->SR & TIM_SR_UIF)
    {
        if(TIM3->CNT < DISTANCE_THRESHOLD)
            distance[distance_meas_idx] = TIM3->CNT;
            distance_meas_idx += 1;
            
            if(distance_meas_idx > DIST_ARRAY_SIZE) 
                distance_meas_idx = 0;
        
        TIM3->SR &= ~(TIM_SR_UIF);
    }
}
