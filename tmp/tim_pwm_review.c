#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gtim.h"
#include "gpio.h"
#include "buzzer.h"

/* Host-only checks: memory at device addresses is ordinary RAM here.
 * This does not simulate clocks, PWM waveforms, preload transfer or rc_w0.
 */
_Static_assert(offsetof(gtim_t, CCMR1) == 0x18, "CCMR1 offset");
_Static_assert(offsetof(gtim_t, CCR1) == 0x34, "CCR1 offset");
_Static_assert(offsetof(gtim_t, CCR4) == 0x40, "CCR4 offset");
_Static_assert(offsetof(gtim_t, DCR) == 0x48, "DCR offset");

static unsigned checks;
static unsigned failures;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
    ++failures; printf("FAIL line %d: %s\n", __LINE__, #condition); } } while (0)

static void seed(gtim_t *tim)
{
    memset(tim, 0xA5, sizeof(*tim));
    tim->CR1 &= ~1U;
}

static void verify_timer(gtim_t *tim)
{
    volatile uint32_t *ccr[] = { &tim->CCR1, &tim->CCR2, &tim->CCR3, &tim->CCR4 };
    seed(tim);
    for (unsigned ch = 0; ch < 4; ++ch)
    {
        gtim_pwm_config_t config = {
            .Mode = (ch % 2U == 0U) ? TIM_PWM_MODE_1 : TIM_PWM_MODE_2,
            .Polarity = (ch % 2U == 0U) ? TIM_POLARITY_HIGH : TIM_POLARITY_LOW,
            .CompareValue = 100U + ch
        };
        gtim_t before;
        memcpy(&before, tim, sizeof(before));
        unsigned shift = (ch % 2U) * 8U;
        uint32_t field_mask = 0x000100FFU << shift;
        uint32_t field_value = ((ch % 2U == 0U) ? 0x68U : 0x78U) << shift;
        CHECK(TIM_PWM_ConfigChannel(tim, (gtim_channel_e)ch, &config) == 0);
        CHECK(*ccr[ch] == 100U + ch);
        CHECK(tim->CNT == before.CNT && tim->PSC == before.PSC && tim->ARR == before.ARR);
        CHECK(tim->EGR == before.EGR && tim->CR1 == before.CR1);
        if (ch < 2U)
        {
            CHECK(tim->CCMR1 == ((before.CCMR1 & ~field_mask) | field_value));
            CHECK(tim->CCMR2 == before.CCMR2);
        }
        else
        {
            CHECK(tim->CCMR2 == ((before.CCMR2 & ~field_mask) | field_value));
            CHECK(tim->CCMR1 == before.CCMR1);
        }
        CHECK(tim->CCER == ((before.CCER & ~(0xBU << (ch * 4U))) |
              ((ch % 2U == 0U) ? 0U : (2U << (ch * 4U)))));
        for (unsigned other = 0; other < 4; ++other)
        {
            if (other != ch)
            {
                const uint32_t old[] = { before.CCR1, before.CCR2, before.CCR3, before.CCR4 };
                CHECK(*ccr[other] == old[other]);
            }
        }
    }

    CHECK(TIM_PWM_Start(tim, TIM_CHANNEL_1) == 0);
    CHECK(TIM_PWM_Start(tim, TIM_CHANNEL_4) == 0);
    CHECK((tim->CCER & 0x1001U) == 0x1001U);
    uint32_t counter = tim->CNT;
    uint32_t event = tim->EGR;
    CHECK(TIM_PWM_Stop(tim, TIM_CHANNEL_1) == 0);
    CHECK((tim->CCER & 1U) == 0U && (tim->CCER & 0x1000U) != 0U);
    CHECK((tim->CR1 & 1U) != 0U && tim->CNT == counter);
    CHECK(TIM_PWM_SetCompare(tim, TIM_CHANNEL_4, 600U) == 0);
    CHECK(tim->CCR4 == 600U && tim->CNT == counter && tim->EGR == event);

    gtim_t before;
    memcpy(&before, tim, sizeof(before));
    gtim_pwm_config_t config = { TIM_PWM_MODE_1, TIM_POLARITY_HIGH, 20U };
    CHECK(TIM_PWM_ConfigChannel(tim, TIM_CHANNEL_2, &config) == -2);
    CHECK(memcmp(tim, &before, sizeof(before)) == 0);
    CHECK(TIM_PWM_ConfigChannel(tim, (gtim_channel_e)4, &config) == -1);
    CHECK(TIM_PWM_ConfigChannel(tim, TIM_CHANNEL_1, NULL) == -1);
    config.Mode = (gtim_pwm_mode_e)8;
    CHECK(TIM_PWM_ConfigChannel(tim, TIM_CHANNEL_1, &config) == -1);
    config.Mode = TIM_PWM_MODE_1;
    config.Polarity = (gtim_polarity_e)-1;
    CHECK(TIM_PWM_ConfigChannel(tim, TIM_CHANNEL_1, &config) == -1);
    CHECK(TIM_PWM_Start(tim, (gtim_channel_e)-1) == -1);
    CHECK(TIM_PWM_Stop(tim, (gtim_channel_e)9) == -1);
    CHECK(TIM_PWM_SetCompare(tim, (gtim_channel_e)5, 0U) == -1);
    CHECK(memcmp(tim, &before, sizeof(before)) == 0);

    GTIMx_Stop(tim);
    config.Polarity = TIM_POLARITY_HIGH;
    config.CompareValue = UINT32_MAX;
    memcpy(&before, tim, sizeof(before));
    if (tim == TIM3 || tim == TIM4)
    {
        CHECK(TIM_PWM_ConfigChannel(tim, TIM_CHANNEL_1, &config) == -1);
        CHECK(TIM_PWM_SetCompare(tim, TIM_CHANNEL_4, 65536U) == -1);
        CHECK(TIM_SetCounter(tim, 65536U) == -1);
        CHECK(GTIM_BaseInit(tim, 0U, 65536U) == -1);
        CHECK(memcmp(tim, &before, sizeof(before)) == 0);
        config.CompareValue = 65535U;
    }
    CHECK(TIM_PWM_ConfigChannel(tim, TIM_CHANNEL_1, &config) == 0);
    CHECK(tim->CCR1 == config.CompareValue);

    memset(tim, 0, sizeof(*tim));
    CHECK(TIM_PWM_Start(tim, TIM_CHANNEL_1) == -2);
    CHECK(TIM_PWM_SetCompare(tim, TIM_CHANNEL_1, 2U) == -2);
    CHECK(tim->CR1 == 0U && tim->CCER == 0U && tim->CCR1 == 0U);
    CHECK(GTIM_BaseInit(tim, 10U, 0U) == -1);
}

int main(void)
{
    void *tim_memory = VirtualAlloc((void *)(uintptr_t)0x40000000U, 0x4000U,
                                   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    void *gpio_memory = VirtualAlloc((void *)(uintptr_t)0x50000000U, 0x10000U,
                                    MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (tim_memory == NULL || gpio_memory == NULL)
    {
        fprintf(stderr, "Cannot map mock register RAM: %lu\n", GetLastError());
        return 1;
    }
    CHECK(Buzzer_Start() == -2 && Buzzer_Stop() == -2);
    verify_timer(TIM2);
    verify_timer(TIM3);
    verify_timer(TIM4);
    verify_timer(TIM5);
    gtim_pwm_config_t config = { TIM_PWM_MODE_1, TIM_POLARITY_HIGH, 5U };
    CHECK(TIM_PWM_ConfigChannel(NULL, TIM_CHANNEL_1, &config) == -1);
    CHECK(TIM_PWM_Start((gtim_t *)(uintptr_t)0x1234U, TIM_CHANNEL_1) == -1);

    seed(TIM2);
    gtim_t timebase_before;
    memcpy(&timebase_before, TIM2, sizeof(timebase_before));
    CHECK(Buzzer_Init() == 0);
    CHECK(TIM4->PSC == 10443U && TIM4->ARR == 9U && TIM4->CCR1 == 5U);
    CHECK((TIM4->CR1 & 1U) == 0U && (TIM4->CCER & 1U) == 0U);
    CHECK(((GPIOB.MODER >> 12U) & 3U) == 1U);
    CHECK(GPIOB.BSRR == (1U << 22U));
    CHECK(Buzzer_Start() == 0);
    CHECK((TIM4->CR1 & 1U) != 0U && (TIM4->CCER & 1U) != 0U);
    CHECK(((GPIOB.MODER >> 12U) & 3U) == 2U && ((GPIOB.AFRL >> 24U) & 15U) == 2U);
    CHECK(Buzzer_Stop() == 0);
    CHECK((TIM4->CCER & 1U) == 0U && (TIM4->CR1 & 1U) != 0U);
    CHECK(((GPIOB.MODER >> 12U) & 3U) == 1U && GPIOB.BSRR == (1U << 22U));
    CHECK(Buzzer_Start() == 0 && Buzzer_Stop() == 0);
    CHECK(memcmp(TIM2, &timebase_before, sizeof(timebase_before)) == 0);

    VirtualFree(tim_memory, 0U, MEM_RELEASE);
    VirtualFree(gpio_memory, 0U, MEM_RELEASE);
    printf("Mock register checks: %u passed, %u failed\n", checks - failures, failures);
    return failures != 0U;
}
