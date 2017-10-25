#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include <string.h>
#include "audio.h"
#include "led.h"

void Error_Handler();

static void init_gpio()
{
    GPIO_InitTypeDef init;

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    init.Pin = GPIO_PIN_13;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init( GPIOC, &init );

    init.Speed = GPIO_SPEED_FREQ_HIGH;
    init.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    init.Mode = GPIO_MODE_AF_PP;

    HAL_GPIO_Init( GPIOA, &init );
}

static void setupCLK(void)
{
    unsigned int StartUpCounter=0;

    // enable HSE
    RCC->CR |= 0x00010001;

    // and wait for it to come on
    while ((RCC->CR & 0x00020000) == 0);

    // enable flash prefetch buffer
    FLASH->ACR = 0x00000012;
	
    // Configure PLL
#ifdef XTAL12M
    RCC->CFGR |= 0x00110400; /* pll=72Mhz(x6),APB1=36Mhz,AHB=72Mhz */
#else
    RCC->CFGR |= 0x001D0400; /* pll=72Mhz(x9),APB1=36Mhz,AHB=72Mhz */
#endif

    // enable the pll
    RCC->CR |= 0x01000000;

#ifndef HSE_STARTUP_TIMEOUT
  #define HSE_STARTUP_TIMEOUT    ((unsigned int)0x0500) /*!< Time out for HSE start up */
#endif /* HSE_STARTUP_TIMEOUT */   

    StartUpCounter = HSE_STARTUP_TIMEOUT;
    while (((RCC->CR & 0x03000000) == 0) && --StartUpCounter);
	
	if (!StartUpCounter) {
		// HSE has not started. Try restarting the processor
		Error_Handler();
	}
	
    // Set SYSCLK as PLL
    RCC->CFGR |= 0x00000002;
    // and wait for it to come on
    while ((RCC->CFGR & 0x00000008) == 0); 

    RCC->AHBENR = 0x0;
    RCC->APB2ENR = 0x1; // AFIO only
}


void Error_Handler()
{
    int c=0;
    while (1) {
        c++;
        HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, c&1);
    }
}

static SPI_HandleTypeDef spih;

static void spi_init()
{
    spih.Instance = SPI1;

    spih.Init.Mode = SPI_MODE_MASTER;
    spih.Init.Direction = SPI_DIRECTION_2LINES;
    spih.Init.DataSize = SPI_DATASIZE_16BIT;
    spih.Init.CLKPolarity = SPI_POLARITY_LOW;
    spih.Init.CLKPhase = SPI_PHASE_1EDGE;
    spih.Init.NSS = SPI_NSS_SOFT;
    spih.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    spih.Init.FirstBit = SPI_FIRSTBIT_MSB;
    spih.Init.TIMode = SPI_TIMODE_DISABLE;
    spih.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spih.Init.CRCPolynomial = 0;

    __HAL_RCC_SPI1_CLK_ENABLE();

    HAL_SPI_Init(&spih);
}

static void spi_tx32(unsigned val)
{
    unsigned char temp[4];
    unsigned char temptx[4];
    uint16_t high = val>>16;
    uint16_t low = val;

    temptx[0] = 0;
    temptx[1] = 0;
    temptx[2] = 0;
    temptx[3] = 0;

    HAL_SPI_TransmitReceive(&spih, &high, temp, 1, 1000);
    HAL_SPI_TransmitReceive(&spih, &low, temp,  1, 1000);
}

int main()
{
    setupCLK();
    SystemCoreClockUpdate();

    HAL_Init();

    init_gpio();
    spi_init();
    audio_init();
    led_init();
    while (1) {
        led_txchunk();
    }

}
