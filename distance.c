#include "distance.h"
#include "defs.h"
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal.h>


static TIM_HandleTypeDef    TimHandle;


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

    return;
    /*Echo Input capture timer and gpio init*/

    //Maximum value up until which the timer counts to. Defines the maximum time that the system waits for an echo.
    // Prescaler = 39 =>  (25 ms)/(1/72E6/40) => 45000  clk ticks   => 0.18 mm of resolution
    uint16_t max_echo = 45000;


    // Enable the necessary low level resources
//    HAL_TIM_IC_MspInit(&TimHandle);

    //Echo GPIO map TIM3_CH1 to PA6 as input

    GPIO_InitTypeDef GPIOinit;

    GPIOinit.Pin = DISTANCE_ECHO_GPIO_PIN;
    GPIOinit.Mode = GPIO_MODE_AF_INPUT;
    GPIOinit.Pull = GPIO_PULLDOWN;
    GPIOinit.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init( DISTANCE_ECHO_GPIO, &GPIOinit );


    // Timer 3 channel 1 input capture configuration
    TIM_IC_InitTypeDef ICHandle;

    ICHandle.ICPolarity =  TIM_ICPOLARITY_RISING;     //Capture a rising edge
    ICHandle.ICSelection = TIM_ICSELECTION_DIRECTTI;   // => IC1
    ICHandle.ICPrescaler = 0 ;
    ICHandle.ICFilter = 0;                          //If there is noise during the beging of capture specify number of rising edges to ignore max of 0xF

    //HAL_TIM_ConfigTI1Input(TIM_HandleTypeDef * htim, uint32_t TI1_Selection)

    HAL_TIM_IC_Init(&TimHandle);
    HAL_TIM_IC_ConfigChannel(&TimHandle, &ICHandle,TIM_CHANNEL_1);  // => PA6

    //Start timer in input capture mode TBD
    HAL_TIM_IC_Start(&TimHandle, TIM_CHANNEL_1);

}






static void distance_delay()
{
    volatile int counter = 80;
    while (counter--) {
    }
}

void distance_ping()
{
    //Send trigger pulse
    //Start timer in input capture mode TBD
    //HAL_TIM_IC_Start(&TimHandle, TIM_CHANNEL_1);

    HAL_GPIO_WritePin( DISTANCE_TRIG_GPIO, DISTANCE_TRIG_GPIO_PIN, 1);
    distance_delay();
    HAL_GPIO_WritePin( DISTANCE_TRIG_GPIO, DISTANCE_TRIG_GPIO_PIN, 0);
}

int distance_read(unsigned *value)
{
    if( __HAL_TIM_GET_FLAG( &TimHandle, TIM_FLAG_CC1) != 0x00)
    {
        return -1;
    }

    //When a rising edge is detected, read the timer counter
    //Distance in meters =  dist/2*(1/(72E6/40) * 343  => 1m = 10496 clock cycles
    *value = HAL_TIM_ReadCapturedValue (&TimHandle, TIM_CHANNEL_1);

    HAL_TIM_IC_Stop(&TimHandle, TIM_CHANNEL_1);

    return 0;

    return -1;
}
