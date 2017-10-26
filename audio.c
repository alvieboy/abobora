#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f1xx_hal_rcc.h>
#include "defs.h"

static TIM_HandleTypeDef    TimHandle;

extern void Error_Handler();
static volatile int bAudioRunning = 0;

void audio_start()
{
    bAudioRunning = 1;
}

void audio_stop()
{
    bAudioRunning = 0;
}

void audio_init()
{
//    TIM_MasterConfigTypeDef master_timer_config;

    /* Set TIMx instance */
    TimHandle.Instance = AUDIO_TIM;

    //APB1=36Mhz,AHB=72Mhz

    // TODO: fix this for correct 8KHz period.

    TimHandle.Init.Period            = 1040;//288;//((144;
    TimHandle.Init.Prescaler         = 0;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;

    AUDIO_TIM_CLK_ENABLE();

    // Disable all interrupts
    __HAL_TIM_DISABLE_IT(&TimHandle, 0x7FFF);

    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
    {
        /* Starting Error */
        Error_Handler();
    }

    __enable_irq();

    HAL_NVIC_SetPriority(AUDIO_TIM_IRQ, 27, 0);
    NVIC_EnableIRQ( AUDIO_TIM_IRQ );
}

extern int get_audio_sample(uint8_t *sample);

void audio_interrupt()
{
    if (bAudioRunning) {
        uint8_t sample;
        if (get_audio_sample(&sample)==0) {
            // Set PWM
        }
    }
}

int get_audio_sample(uint8_t *val)
{
    return -1;
}