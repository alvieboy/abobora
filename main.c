#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include <usbd_cdc.h>
#include "usbd_cdc_if.h"
#include <string.h>
#include "audio.h"
#include "led.h"
#include "distance.h"
#include "fogo-pallete.h"
#include "spi.h"
#include "spiflash.h"
#include "timer.h"

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
        volatile int z=900000;
        while (z--) {}
    }
}


static USBD_HandleTypeDef  USBD_Device;
extern USBD_DescriptorsTypeDef VCP_Desc;

void usb_init()
{
//    usb_init_pins();

    USBD_Init(	&USBD_Device, &VCP_Desc, 0);
    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

    /* Add CDC Interface Class */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

    /* Start Device Process */
    USBD_Start(&USBD_Device);
}


void usb__transmit(uint8_t *buf, int len)
{
    USBD_CDC_SetTxBuffer( &USBD_Device, buf, len);
    while (USBD_CDC_TransmitPacket( &USBD_Device ) == USBD_BUSY) {
        //delay_ms(1);
    }
}
void usb_init_pins()
{
  GPIO_InitTypeDef  GPIO_InitStruct;
   
  /* Enable the GPIOA clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* Configure USB DM/DP pins */
  GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
  GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


#if 0

  /* Enable the USB disconnect GPIO clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  /* USB_DISCONNECT used as USB pull-up */
  GPIO_InitStruct.Pin = USB_DISCONNECT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  HAL_GPIO_Init(USB_DISCONNECT_PORT, &GPIO_InitStruct);
#endif

  /* Enable USB Clock */
  __HAL_RCC_USB_CLK_ENABLE();
#if 0
  if (hpcd->Init.low_power_enable == 1)
  {
    /* Enable EXTI for USB wakeup */
    __HAL_USB_WAKEUP_EXTI_CLEAR_FLAG();
    __HAL_USB_WAKEUP_EXTI_ENABLE_RISING_EDGE();
    __HAL_USB_WAKEUP_EXTI_ENABLE_IT();
    
    /* USB Wakeup Interrupt */
    HAL_NVIC_EnableIRQ(USBWakeUp_IRQn);
    
    /* Enable USB Wake-up interrupt */
    HAL_NVIC_SetPriority(USBWakeUp_IRQn, 0, 0);
  }
#endif
  /* Set USB Interrupt priority */
  HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);

  /* Enable USB Interrupt */
  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

static uint8_t lv=0;
void callback_end_of_led_frame()
{
    int i;
    for (i=0;i<LED_NUMBER;i++)
        led_setpixel(i, lv);
    lv++;

}

int main()
{
    setupCLK();
    SystemCoreClockUpdate();

    HAL_Init();

    init_gpio();

    spi_init();
    spiflash_init();
    //usb_init();

    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, 0);
    servo_init();

    //audio_init();

    led_init();
    led_setpallete(fogo_pallete);


    distance_init();


    volatile int z = 0;
    while (1) {
       // HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, 1);
        led_txchunk();
        // HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, 0);
        spiflash_check();
        timer__iterate();
        z++;
#if 0
        if ((z==64000)) {
            z=0;
            distance_read();
        }
#endif
    }

}

// FOR SPI. Map to UART or USB/UART

void outbyte(uint8_t val)
{
}

int inbyte(uint8_t *target)
{
    return -1;
}
