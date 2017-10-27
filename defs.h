#ifndef __DEFS_H__
#define __DEFS_H__


// Audio timer settings

#define AUDIO_TIM TIM2
#define AUDIO_TIM_CLK_ENABLE __HAL_RCC_TIM2_CLK_ENABLE
#define AUDIO_TIM_IRQ TIM2_IRQn

// LED
//#define LED_CHUNK 8
#define LED_NUMBER 96

#define LED_DATA_GPIO GPIOB
#define LED_CLK_GPIO GPIOB

#define LED_DATA_GPIO_PIN GPIO_PIN_10
#define LED_CLK_GPIO_PIN GPIO_PIN_11

// Distance sensor

#define DISTANCE_TRIG_GPIO_PIN GPIO_PIN_3
#define DISTANCE_TRIG_GPIO GPIOB
#define DISTANCE_TRIG_GPIO_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE

// SPI/SPI Flash
#define SPI_CS_GPIO GPIOA
#define SPI_CS_GPIO_PIN 15
#define SPI_GPIO_CLK_ENABLE() /* Already done */

#endif