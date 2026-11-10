#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define  RCC_MP_AHB5ENSETR (*(volatile unsigned int *)0x50000210)

#define GPIOZ (*(volatile gpio_t *)0x54004000)

#define GPIO_PIN_0 0x0001
#define GPIO_PIN_1 0x0002
#define GPIO_PIN_2 0x0004
#define GPIO_PIN_3 0x0008
#define GPIO_PIN_4 0x0010
#define GPIO_PIN_5 0x0020
#define GPIO_PIN_6 0x0040
#define GPIO_PIN_7 0x0080
#define GPIO_PIN_8 0x0100
#define GPIO_PIN_9 0x0200
#define GPIO_PIN_10 0x0400
#define GPIO_PIN_11 0x0800
#define GPIO_PIN_12 0x1000
#define GPIO_PIN_13 0x2000
#define GPIO_PIN_14 0x4000
#define GPIO_PIN_15 0x8000

typedef struct
{
    volatile unsigned int MODER;
    volatile unsigned int OTYPER;
    volatile unsigned int OSPEEDR;
    volatile unsigned int PUPDR;
    volatile unsigned int IDR;
    volatile unsigned int ODR;
    volatile unsigned int BSRR;
}gpio_t;

typedef enum 
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
}gpio_state_e;

typedef enum
{
    GPIO_INPUT_MODE = 0,
    GPIO_OUTPUT_MODE,
    GPIO_AF_MODE,
    GPIO_ANALOG_MODE
}gpio_mode_e;

typedef enum
{
    GPIO_OUTPUT_PP = 0,
    GPIO_OUTPUT_OD
}gpio_otyper_e;

typedef enum
{
    GPIO_NOPPNOPD = 0,
    GPIO_PULLUP,
    GPIO_PULLDOWN,
}gpio_pupdr_e;

typedef enum
{
    GPIO_LOW_SPEED = 0,
    GPIO_MEDIUM_SPEED,
    GPIO_HIGH_SPEED,
    GPIO_VERY_HIGH_SPEED
}gpio_ospeedr_e;

typedef struct 
{
    gpio_mode_e Mode;
    gpio_otyper_e Otype;
    gpio_ospeedr_e Speed;
    uint16_t Pin;
    gpio_pupdr_e pupd;
    gpio_state_e State;
}gpioinit_t;

void HAL_GPIO_Init(volatile gpio_t *GPIOx, const gpioinit_t *GPIO_Init);
uint8_t HAL_GPIO_Write(volatile gpio_t *GPIOx, uint16_t _pin, gpio_state_e _val);
uint8_t HAL_GPIO_Read(volatile gpio_t *GPIOx, uint16_t _pin);
uint8_t HAL_GPIO_Toggle(volatile gpio_t *GPIOx, uint16_t _pin);

#endif
