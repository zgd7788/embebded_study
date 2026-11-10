#include "gpio.h"
#include <stddef.h>
/*
    功能：根据配置参数初始化指定 GPIO 引脚
    参数：
        @GPIOx：指向目标 GPIO 端口寄存器组的指针
        @GPIO_Init：指向初始化配置结构体的只读指针，
                    包含引脚、模式、输出类型、速度、
                    上下拉和初始输出电平
    前置条件：调用者已经开启目标 GPIO 端口的时钟
    返回值：无
*/
void HAL_GPIO_Init(volatile gpio_t *GPIOx, const gpioinit_t *GPIO_Init)
{
    if((GPIOx == NULL)||(GPIO_Init == NULL))
    {
        return;
    }
    if (GPIO_Init->Pin == 0U)
    {
        return;
    }
    if((GPIO_Init->Mode > 3U)||(GPIO_Init->Otype > 1U)||
        (GPIO_Init->pupd > 2U) ||(GPIO_Init->Speed > 3U)||(GPIO_Init->State > 1U))
    {
        return;
    }
    uint16_t _pin;
    uint8_t index;
    uint8_t shift;

    for (index = 0U; index < 16U; index++)
    {
        _pin = 1U << index;
        if(((1U<<index) & GPIO_Init->Pin) != 0U)
        {
            shift = (uint32_t)index * 2U;
            switch (GPIO_Init->Mode)
            {
            case GPIO_INPUT_MODE:
                GPIOx->PUPDR &= ~(3U << shift);
                GPIOx->PUPDR |= (uint32_t)(GPIO_Init->pupd << (shift));
                GPIOx->MODER &= ~(3U << shift);
                GPIOx->MODER |= (GPIO_INPUT_MODE << shift);
                break;
            case GPIO_OUTPUT_MODE:
                GPIOx->OSPEEDR &= ~(3U << shift);
                GPIOx->OSPEEDR |= ((uint32_t)GPIO_Init->Speed << shift);
                GPIOx->OTYPER &= ~(1U << index);
                GPIOx->OTYPER |= ((uint32_t)GPIO_Init->Otype << index);
                if(GPIO_Init->State == GPIO_PIN_RESET)
                {
                    GPIOx->BSRR = ((uint32_t)_pin << 16U);
                }
                else
                {
                    GPIOx->BSRR = (uint32_t)_pin;
                }
                GPIOx->MODER &= ~(3U << shift);
                GPIOx->MODER |= (GPIO_OUTPUT_MODE << shift);
                break;
            case GPIO_AF_MODE:
                break;
            case GPIO_ANALOG_MODE:
                GPIOx->MODER &= ~(3U << shift);
                GPIOx->MODER |= (GPIO_ANALOG_MODE << shift);
                break;
            default:
                break;
            }
            
        }
        
    }
}
/*
    功能：控制GPIO输出电平
    参数：
    返回值：成功返回0，失败返回1
*/
uint8_t HAL_GPIO_Write(volatile gpio_t *GPIOx, uint16_t _pin, gpio_state_e _val)
{
    if((GPIOx == NULL)||(_pin == 0U)||(_val > GPIO_PIN_SET)
        ||(_pin&(_pin-1u)) != 0U)
    {
        return 1U;
    }
    if(_val == 1U)
    {
        GPIOx->ODR |= _pin; 
    }
    else
    {
        GPIOx->ODR &= ~_pin; 
    }
    return 0U;
}
/*
    功能：读取GPIO电平
    参数：
    返回值：成功返回读取到的电平，失败返回0，目前失败和读到0无法区分
*/
uint8_t HAL_GPIO_Read(volatile gpio_t *GPIOx, uint16_t _pin)
{
    if((GPIOx == NULL)||(_pin == 0U)||(_pin&(_pin-1U))!=0U)
    {
        return 0U;
    }
    return ((GPIOx->IDR & _pin) != 0U) ? 1U:0U;
}
/*
    功能：反转指定GPIO引脚
    参数：
    返回值：成功返回0，失败返回1
*/
uint8_t HAL_GPIO_Toggle(volatile gpio_t *GPIOx, uint16_t _pin)
{
    if((GPIOx == NULL)||(_pin == 0U)||(_pin&(_pin-1U))!=0U)
    {
        return 1U;
    }
    if((GPIOx->ODR & _pin) != 0U)
    {
        GPIOx->BSRR = ((uint32_t)_pin << 16U);
    }
    else
    {
        GPIOx->BSRR = (uint32_t)_pin;
    }
    return 0;
}
