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
#include "fogo-pallete2.h"
#include "spi.h"
#include "spiflash.h"
#include "timer.h"
#include "servo.h"
#include "uart.h"
#include "engine.h"

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

    init.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init( GPIOB, &init );

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
        volatile int z=9000;
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


unsigned short lfsr = 0xACE1u;
unsigned bit;

unsigned myrand()
{
    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    return lfsr =  (lfsr >> 1) | (bit << 15);
}

static uint8_t lv=0;

void callback_end_of_led_frame()
{
    volatile uint8_t r = myrand() & 0xff;
    int i;

    lv++;
    if ((lv & 0x3F) == (0x3F))
    {
        main_state_t state = engine_get_state();
        switch (state) {
        case ENGINE_IDLE:
        default:

            for (i=0;i<71;i++) {
                led_setpixel(i,r);
            }
            led_setpixel(0, lv);
            led_setpixel(0, lv);
            led_setpixel(0, lv);
            led_setpixel(0, lv);

            break;
        case ENGINE_APPROACH:

            for (i=0;i<71;i++) {
                led_setpixel(i,0xff);
            }

            led_setpixel(72, lv);
            led_setpixel(73, lv);
            led_setpixel(74, lv);
            led_setpixel(75, lv);

            break;
        }
    }

    lv++;

#if 0
    int i;
    for (i=0;i<LED_NUMBER;i++)
        led_setpixel(i, lv);
    lv++;
#endif
}

#define SERVO_LID_CLOSED 850
#define SERVO_LID_OPEN   3500

#define SERVO_EYE_OUT    900
#define SERVO_EYE_IN     2580

static void close_lid()
{
    servo_enable();
    servo_set_channel_a(SERVO_LID_CLOSED);
//    servo_disable();
}

static void open_lid()
{
    servo_enable();
    servo_set_channel_a(SERVO_LID_OPEN);
//    servo_disable();
}

static void eye_out()
{
    servo_enable();
    servo_set_channel_b(SERVO_EYE_OUT);
//    servo_disable();
}

static void eye_in()
{
    servo_enable();
    servo_set_channel_b(SERVO_EYE_IN);
//    servo_disable();
}

enum {
    CLOSED,
    OPEN_LID,
    EYE_OUT,
    EYE_ROTATE,
    EYE_IN,
    CLOSE_LID
} tampa = CLOSED;

int ttimer = -1;


int effect_finished()
{
    switch (tampa) {
    case OPEN_LID:
        eye_out();
        tampa = EYE_OUT;
        break;
    case EYE_OUT:
        rotate_eye();
        tampa = EYE_ROTATE;
        break;
    case EYE_ROTATE:
        eye_in();
        tampa = EYE_IN;
        break;
    case EYE_IN:
        close_lid();
        tampa = CLOSE_LID;
        break;
    case CLOSE_LID:
        tampa = CLOSED;
        return -1;
        break;

    }
    return 0;
}


int motorlr = 0;
int motorcnt=0;
int do_rotate_eye(void *data)
{
    HAL_GPIO_WritePin( GPIOB, GPIO_PIN_5, motorlr);
    HAL_GPIO_WritePin( GPIOB, GPIO_PIN_6, !motorlr);

    motorlr = ! motorlr;
    motorcnt++;
    if (motorcnt<10)
        return 0;

    HAL_GPIO_WritePin( GPIOB, GPIO_PIN_5, 0);
    HAL_GPIO_WritePin( GPIOB, GPIO_PIN_6, 0);
    return -1;

}

void rotate_eye()
{
    motorcnt=0;
    ttimer = timer__add(200, &do_rotate_eye, 0);

}

void effect()
{
    switch(tampa) {
    case CLOSED:
        tampa = OPEN_LID;
        ttimer = timer__add(2000, &effect_finished, 0);
        open_lid();
        break;
    default:
        break;

    }
}



static uint8_t ledv=0;

int led_toggle(void *data)
{
    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, ledv);
    ledv = !ledv;
    return 0;
}

static unsigned int sensor_tick = 0;
int ones_count = 0;

int sensor_proximity()
{
    return ones_count==0;
}

int get_distance()
{
    return sensor_proximity() ? 2 : 3;
}

int sensor_ping(void *data)
{
    if (sensor_tick==0) {
        // ping
        distance_ping();
    }

    if (sensor_tick==8) {
        int v = distance_read_echo();
        outstring("V ");
        printhex(v);
        outstring("\r\n");
        if (v==1) {
            if (ones_count<3)
                ones_count++;
        } else {
            ones_count=0;
#if 0
            if (ones_count>0)
                ones_count--;
#endif
        }
    }

    sensor_tick++;
    if (sensor_tick==100)
        sensor_tick = 0;


    //outstring("\xff");
    return 0;
}

#if 0
int motorlr = 0;
int motor_ctl(void *data)
{
    HAL_GPIO_WritePin( GPIOB, GPIO_PIN_5, motorlr);
    HAL_GPIO_WritePin( GPIOB, GPIO_PIN_6, !motorlr);

    motorlr = ! motorlr;
    return 0;
}
#endif
int engine_tick()
{
    engine_loop();
    return 0;
}

int main()
{
    setupCLK();
    SystemCoreClockUpdate();

    HAL_Init();

    init_gpio();
    uart_init();
    outstring("Ready to rock!!\r\n");
    //outbyte("C");

    spi_init();
    spiflash_init();
    distance_init();
    engine_init();
    //usb_init();

    // SPI test
    spi_disable();
    spi_enable();

    unsigned id = 0;
    spi_write(0x9f);
    spi_write(0x00);
    id = spi_read();
    spi_write(0x00);
    id<<=8;
    id += spi_read();
    spi_write(0x00);
    id<<=8;
    id = spi_read();
    outstring("SPI ID: ");
    printhex(id);
    outstring("\r\n");

    spi_disable();

    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, 0);
    servo_init();
#if 0
    while (1) {
        open_lid();
        eye_out();
        eye_in();
        close_lid();
    }
    // Close LID
//    close_lid();
    eye_out();
    //audio_init();
#endif

    servo_disable();
    led_init();
    led_setpallete(fogo_pallete);

    int i;
    for (i=0;i<72;i++)
        led_setpixel(i, i*2);
    led_setpixel(72, 0);
    led_setpixel(73, 0);
    led_setpixel(74, 0);
    led_setpixel(75, 0);


    //distance_init();

    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, 1);

    timer__init();
    timer__add(1, &sensor_ping, NULL);
    timer__add(100, &led_toggle, NULL);
    timer__add(500, &engine_tick, NULL);


    volatile int z = 0;
    while (1) {
        //HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, 1);
        led_txchunk();
        //HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, 0);
        //spiflash_check();
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

