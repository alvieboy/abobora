#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f1xx_hal_rcc.h>
#include "defs.h"


static TIM_HandleTypeDef    TimHandle;
static TIM_OC_InitTypeDef sConfig;


#define AUDIO_PERIOD         256
#define AUDIO_PRESCALER           1
#define SAMPLE_REPLICATION        18

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

    GPIO_InitTypeDef   GPIO_InitStruct;
    /* TIMx Peripheral clock enable */
    AUDIO_TIM_CLK_ENABLE();

    /* Enable all GPIO Channels Clock requested */
    AUDIO_TIM_CHANNEL_GPIO_PORT();

    /* Configure PA.0 */
    /* Common configuration for all channels */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = AUDIO_TIM_GPIO_PIN_CHANNEL1;
    HAL_GPIO_Init(AUDIO_TIM_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);

    /* Set TIMx instance */
    TimHandle.Instance = AUDIO_TIM;


    //APB1=36Mhz,AHB=72Mhz


    TimHandle.Init.Period            = AUDIO_PERIOD;
    TimHandle.Init.Prescaler         = AUDIO_PRESCALER;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;
    
    

    if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
  
    /* Configure the PWM channels #########################################*/
    /* Common configuration for all channels */
    sConfig.OCMode       = TIM_OCMODE_PWM1;
    sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

    /* Set the pulse value for channel 1 */
    sConfig.Pulse = 0;
    if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
        /* Configuration Error */
        Error_Handler();
    }
  
  
    /*##-Start PWM signals generation #######################################*/
    /* Start channel 1 */
    if (HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1) != HAL_OK)
    {
        /* PWM Generation Error */
        Error_Handler();
    }

    // Disable all interrupts
    __HAL_TIM_DISABLE_IT(&TimHandle, 0x7FFF);

    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
        // Initialization Error 
        Error_Handler();
    }

    if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
    {
        // Starting Error 
        Error_Handler();
    }

    __enable_irq();

    HAL_NVIC_SetPriority(AUDIO_TIM_IRQ, 27, 0);
    NVIC_EnableIRQ( AUDIO_TIM_IRQ );
    
    //AUDIO_TIM->CCMR1 &= ~0x8; //Don't use CCR shadow value
    
}

extern int get_audio_sample(uint8_t *sample);

uint16_t sample_replication_idx = 0;
uint8_t ccr = 0;

void audio_interrupt()
{
    if (bAudioRunning) {
        if(TIM2->SR & TIM_SR_UIF)
        { 
            if(sample_replication_idx == 0) // Update CCR for every set of SAMPLE_REPLICATION samples
                if(get_audio_sample(&ccr) != 0) //Get the next sample else set ouput to 0
                    ccr = 0;
            
            sample_replication_idx += 1;
            
            if(sample_replication_idx >= SAMPLE_REPLICATION)
                sample_replication_idx = 0;

            AUDIO_TIM->CCR1 = ccr; // Update CCR value
            //AUDIO_TIM->EGR |= 1; // Should force the CCR value to udpate.
                
            /*
             * Clears the interrupt event
             */
            TIM2->SR &= ~(TIM_SR_UIF);
        }
    }
}



int get_audio_sample(uint8_t *val)
{
    return -1;
}
