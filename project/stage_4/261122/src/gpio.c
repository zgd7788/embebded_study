#include "gpio.h"
#include <stddef.h>
/*
    功能：根据配置参数初始化 GPIO 端口中的一个或多个引脚
    参数：
        @GPIOx：指向目标 GPIO 端口寄存器组的指针
        @GPIO_Init：指向初始化配置结构体的只读指针，
                    包含引脚、模式、输出类型、速度、
                    上下拉、初始输出电平和复用功能编号
    前置条件：调用者已经开启目标 GPIO 端口的时钟
    返回值：无；空指针、Pin 为 0 或本函数检查到的配置字段超出范围时不进行初始化
    说明：GPIOx 必须指向目标端口的寄存器组；Pin 可用位掩码同时选择多个引脚
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
        (GPIO_Init->Pupd > 2U) ||(GPIO_Init->Speed > 3U)||(GPIO_Init->State > 1U)||
        (GPIO_Init->AlternateFunc > 15U))
    {
        return;
    }
    uint16_t _pin;
    uint8_t index;
    uint8_t shift;
    uint8_t af_shift;

    for (index = 0U; index < 16U; index++)
    {
        _pin = 1U << index;
        if(((1U<<index) & GPIO_Init->Pin) != 0U)
        {
            shift = (uint32_t)index * 2U;
            af_shift = ((uint32_t)index % 8) * 4U;
            switch (GPIO_Init->Mode)
            {
            case GPIO_INPUT_MODE:
                GPIOx->PUPDR &= ~(3U << shift);
                GPIOx->PUPDR |= (uint32_t)(GPIO_Init->Pupd << (shift));
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
                if(index < 8U)
                {
                    GPIOx->AFRL &= ~(0xFU << af_shift);
                    GPIOx->AFRL |= ((uint32_t)GPIO_Init->AlternateFunc << af_shift);
                }
                else
                {
 
                    GPIOx->AFRH &= ~(0xFU << af_shift); 
                    GPIOx->AFRH |= ((uint32_t)GPIO_Init->AlternateFunc << af_shift);
                }
                GPIOx->OTYPER &= ~(1U << index);
                GPIOx->OTYPER |= ((uint32_t)GPIO_Init->Otype << index);

                GPIOx->OSPEEDR &= ~(3U << shift);
                GPIOx->OSPEEDR |= ((uint32_t)GPIO_Init->Speed << shift);

                GPIOx->PUPDR &= ~(3U << shift);
                GPIOx->PUPDR |= ((uint32_t)GPIO_Init->Pupd << shift);

                GPIOx->MODER &= ~(3U << shift);
                GPIOx->MODER |= (GPIO_AF_MODE << shift);
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
    功能：控制 GPIO 指定引脚的输出电平
    参数：
        @GPIOx：指向目标 GPIO 端口寄存器组的指针
        @_pin：目标引脚掩码，必须恰好选择一个引脚
        @_val：GPIO_PIN_RESET 或 GPIO_PIN_SET，分别表示输出低电平或高电平
    前置条件：目标引脚已配置为输出模式，且 GPIO 端口时钟已开启
    返回值：成功返回 0；GPIOx 为空、引脚掩码无效或 _val 大于 GPIO_PIN_SET 返回 1
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
    功能：读取 GPIO 指定引脚的输入电平
    参数：
        @GPIOx：指向目标 GPIO 端口寄存器组的指针
        @_pin：目标引脚掩码，必须恰好选择一个引脚
    前置条件：GPIO 端口时钟已开启；引脚模式应适合读取输入电平
    返回值：成功时返回 0 或 1 表示引脚电平；参数无效时返回 0，故无法与有效低电平区分
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
    功能：反转 GPIO 指定引脚当前的输出状态
    参数：
        @GPIOx：指向目标 GPIO 端口寄存器组的指针
        @_pin：目标引脚掩码，必须恰好选择一个引脚
    前置条件：目标引脚已配置为输出模式，且 GPIO 端口时钟已开启
    返回值：成功返回 0；GPIOx 为空或引脚掩码无效返回 1
    说明：根据 ODR 当前值，通过 BSRR 将引脚置位或复位
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
