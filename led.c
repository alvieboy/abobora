#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f1xx_hal_rcc.h>
#include "led.h"


// Max 50mA/LED

#ifdef LED_CHUNK
static indexed_pixel_t led_framebuffer[LED_CHUNK];
#else
static indexed_pixel_t led_framebuffer[LED_NUMBER];
#endif

static pixel_t *pallete = NULL;

static unsigned ledptr = 0;

void led_setpallete(pixel_t *pal)
{
    pallete=pal;
}

static pixel_t led_pixel_from_idx(uint8_t idx)
{
    if (pallete) {
        return pallete[idx];
    }
    return 0xffffff;
}

void led_init()
{
    // TBD. GPIO setup.

    GPIO_InitTypeDef init;

    init.Pin = LED_CLK_GPIO_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init( LED_CLK_GPIO, &init );

    init.Pin = LED_DATA_GPIO_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init( LED_DATA_GPIO, &init );

    ledptr = 0;
}

static void led_txbit(unsigned val)
{
    HAL_GPIO_WritePin( LED_DATA_GPIO, LED_DATA_GPIO_PIN, val);
    HAL_GPIO_WritePin( LED_CLK_GPIO, LED_CLK_GPIO_PIN, 1);
    // Delay?
    HAL_GPIO_WritePin( LED_CLK_GPIO, LED_CLK_GPIO_PIN, 0);
}

static void led_transmit(uint32_t data)
{
    // MSB-first
    unsigned i;
    for (i=0;i<32;i++) {
        unsigned val = !!(data & 0x80000000);
        data<<=1;
        led_txbit(val);
    }
}

void led_flush()
{
    // Send one bit per LED
    unsigned i;
    for (i=0; i<(LED_NUMBER/8/4)+1; i++) {
        led_transmit(0x0);
    }
}

void led_txchunk()
{
    unsigned chunksize = 8;

    while (chunksize && (ledptr<LED_NUMBER)) {
#ifdef LED_CHUNK
        unsigned ledpos = ledptr & (LED_CHUNK-1);
#else
        unsigned ledpos = ledptr;
#endif
        indexed_pixel_t pixelidx = led_framebuffer[ledpos];
        pixel_t pixel = led_pixel_from_idx(pixelidx);
        pixel |= 0xFF000000; // Setup data frame, max brightness
        led_transmit(pixel);
        chunksize--;
        ledptr++;
    }
    if (ledptr>=LED_NUMBER) {
        ledptr=0;
        led_flush();
    }
}
