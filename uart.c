#include "uart.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_gpio.h"
#include "defs.h"
#include <string.h>

static UART_HandleTypeDef UartHandle;


void uart_init()
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    USARTx_CLK_ENABLE();

    GPIO_InitStruct.Pin       = USARTx_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = USARTx_RX_PIN;

    HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

    UartHandle.Instance        = USARTx;

    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
}

void outbyte(int c)
{
    uint8_t ch = c;
    while (HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF)!=HAL_OK);
}

void outstring(const char *str)
{
    while (*str) {
        unsigned char c = *str;
        while (HAL_UART_Transmit(&UartHandle, &c, 1, 0xFFFF)!=HAL_OK);
        str++;
    }
}

int inbyte(unsigned char *c)
{
    if(HAL_UART_Receive(&UartHandle, c, 1, 0) != HAL_OK)
    {
        return -1;
    }
    return 0;

}


void printnibble(unsigned int c)
{
    c&=0xf;
    if (c>9)
        outbyte(c+'a'-10);
    else
        outbyte(c+'0');
}

void printhexbyte(unsigned int c)
{
    printnibble(c>>4);
    printnibble(c);
}


void printhex(unsigned int c)
{
    printhexbyte(c>>24);
    printhexbyte(c>>16);
    printhexbyte(c>>8);
    printhexbyte(c);
}
