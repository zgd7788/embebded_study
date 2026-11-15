/* brief: 实现主板LED1灯的闪烁 */

#include "gpio.h"

#define LED_PIN GPIO_PIN_5
#define DELAY_LOOP_COUNT 80000000U

static void delay_roughly(void)
{
    volatile uint32_t count = DELAY_LOOP_COUNT;

    while (count > 0U)
    {
        count--;
    }
}

int main(void)
{
    const gpioinit_t led_config = {
        .Mode = GPIO_OUTPUT_MODE,
        .Otype = GPIO_OUTPUT_PP,
        .Speed = GPIO_LOW_SPEED,
        .Pin = LED_PIN,
        .pupd = GPIO_NOPPNOPD,
        .State = GPIO_PIN_RESET
    };

    RCC_MP_AHB5ENSETR = (1U << 0);
    HAL_GPIO_Init(&GPIOZ, &led_config);

    for (;;)
    {
        (void)HAL_GPIO_Toggle(&GPIOZ, LED_PIN);
        delay_roughly();
    }
}
