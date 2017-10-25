#ifndef __DEFS_H__
#define __DEFS_H__


// Audio timer settings

#define AUDIO_TIM TIM2
#define AUDIO_TIM_CLK_ENABLE __HAL_RCC_TIM2_CLK_ENABLE
#define AUDIO_TIM_IRQ TIM2_IRQn

#define LED_CHUNK 8
#define LED_NUMBER 64

#define LED_DATA_GPIO GPIOB
#define LED_CLK_GPIO GPIOB

#define LED_DATA_GPIO_PIN GPIO_PIN_10
#define LED_CLK_GPIO_PIN GPIO_PIN_11




#endif