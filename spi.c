#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include "defs.h"
#include "spi.h"

static SPI_HandleTypeDef spih;

void spi_init()
{
    spih.Instance = SPI1;
    GPIO_InitTypeDef init;

    spih.Init.Mode = SPI_MODE_MASTER;
    spih.Init.Direction = SPI_DIRECTION_2LINES;
    spih.Init.DataSize = SPI_DATASIZE_8BIT;
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


    SPI_GPIO_CLK_ENABLE();

    init.Pin = SPI_CS_GPIO_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init( SPI_CS_GPIO, &init );

}

unsigned char rval;

void spi_write(unsigned char val)
{
    HAL_SPI_TransmitReceive(&spih, &val, &rval, 1, 1000);
}

void spi_write32(uint32_t val)
{
    spi_write(val>>24);
    spi_write(val>>16);
    spi_write(val>>8);
    spi_write(val);
}

uint8_t spi_read()
{
    return rval;
}

void spi_enable()
{
    HAL_GPIO_WritePin( SPI_CS_GPIO, SPI_CS_GPIO_PIN, 0);
}

void spi_disable()
{
    HAL_GPIO_WritePin( SPI_CS_GPIO, SPI_CS_GPIO_PIN, 1);
}
