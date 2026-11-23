/* Host-only event model. Runs the actual driver against RAM at the device
 * address and advances selected hardware events at timebase reads. It checks
 * control flow, not electrical timing, W1C behavior, or codec compatibility. */
#include <windows.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"
#include "timebase.h"

_Static_assert(offsetof(i2c_t, ISR) == 0x18U, "ISR offset");
_Static_assert(offsetof(i2c_t, RXDR) == 0x24U, "RXDR offset");
_Static_assert(offsetof(i2c_t, TXDR) == 0x28U, "TXDR offset");

enum scenario {
    NORMAL, SIMULTANEOUS_RX_STOP, ADDRESS_NACK, REGISTER_NACK,
    READ_NACK, WRITE_EARLY_STOP, READ_EARLY_STOP, BUSY_TIMEOUT,
    TXIS_TIMEOUT, TC_TIMEOUT, RXNE_TIMEOUT, STOP_TIMEOUT,
    BUSY_THEN_FREE, NACK_WITH_DATA, NACK_WITH_TC
};
static enum scenario scenario;
static unsigned phase, polls, writes, restarts, checks, failures;
static uint32_t tick;
static uint8_t expected_addr, expected_reg, response;

#define CHECK(c) do { ++checks; if (!(c)) { ++failures; \
    printf("FAIL scenario=%d phase=%u line=%d: %s\n", \
           (int)scenario, phase, __LINE__, #c); } } while (0)

static void check_transfer(int read_direction)
{
    const uint32_t cr2 = I2C5->CR2;
    CHECK((cr2 & I2C_CR2_START_Msk) != 0U);
    CHECK((cr2 & I2C_CR2_STOP_Msk) == 0U);
    CHECK((cr2 & I2C_CR2_SADD_Msk) == ((uint32_t)expected_addr << 1U));
    CHECK((cr2 & I2C_CR2_NBYTES_Msk) == (1U << I2C_CR2_NBYTES_Pos));
    CHECK((cr2 & (I2C_CR2_ADD10_Msk | I2C_CR2_RELOAD_Msk)) == 0U);
    CHECK(((cr2 & I2C_CR2_RD_WRN_Msk) != 0U) == read_direction);
    CHECK(((cr2 & I2C_CR2_AUTOEND_Msk) != 0U) == read_direction);
    I2C5->CR2 &= ~I2C_CR2_START_Msk; /* hardware sends address */
}

uint32_t Timebase_GetTicks(gtim_t *timer)
{
    CHECK(timer == TIM2);
    if (++polls > 100U)
    {
        fprintf(stderr, "poll limit exceeded\n");
        exit(2);
    }

    if (scenario == BUSY_TIMEOUT)
    {
        return tick++;
    }
    if ((scenario == BUSY_THEN_FREE) && (polls < 4U))
    {
        I2C5->ISR = I2C_ISR_BUSY_Msk;
        return tick++;
    }
    if ((scenario == BUSY_THEN_FREE) && (polls == 4U))
    {
        I2C5->ISR = 0U;
    }

    switch (phase)
    {
        case 0:
            if ((I2C5->CR2 & I2C_CR2_START_Msk) == 0U)
            {
                break;
            }
            check_transfer(0);
            CHECK(I2C5->ICR == (I2C_ICR_STOPCF_Msk | I2C_ICR_NACKCF_Msk));
            I2C5->ISR = I2C_ISR_BUSY_Msk;
            if (scenario == ADDRESS_NACK)
            {
                I2C5->ISR |= I2C_ISR_NACKF_Msk;
                phase = 5U;
            }
            else if (scenario == WRITE_EARLY_STOP)
            {
                I2C5->ISR = I2C_ISR_STOPF_Msk;
                phase = 6U;
            }
            else if (scenario == TXIS_TIMEOUT)
            {
                phase = 7U;
            }
            else
            {
                I2C5->ISR |= I2C_ISR_TXIS_Msk;
                phase = 1U;
            }
            break;

        case 1:
            CHECK(I2C5->TXDR == expected_reg);
            CHECK((I2C5->CR2 & (I2C_CR2_START_Msk | I2C_CR2_RD_WRN_Msk |
                                I2C_CR2_AUTOEND_Msk)) == 0U);
            ++writes;
            I2C5->ISR = I2C_ISR_BUSY_Msk;
            if ((scenario == REGISTER_NACK) || (scenario == NACK_WITH_TC))
            {
                I2C5->ISR |= I2C_ISR_NACKF_Msk;
                if (scenario == NACK_WITH_TC)
                {
                    I2C5->ISR |= I2C_ISR_TC_Msk;
                }
                phase = 5U;
            }
            else
            {
                phase = (scenario == TC_TIMEOUT) ? 7U : 8U;
            }
            break;

        case 8: /* one tick after TXDR submission: the byte is now complete */
            CHECK((I2C5->CR2 & (I2C_CR2_START_Msk | I2C_CR2_STOP_Msk |
                                I2C_CR2_RD_WRN_Msk)) == 0U);
            I2C5->ISR = I2C_ISR_BUSY_Msk | I2C_ISR_TC_Msk;
            phase = 2U;
            break;

        case 2:
            CHECK((I2C5->ISR & I2C_ISR_BUSY_Msk) != 0U);
            check_transfer(1);
            ++restarts;
            I2C5->ISR = I2C_ISR_BUSY_Msk;
            if (scenario == READ_NACK)
            {
                I2C5->ISR |= I2C_ISR_NACKF_Msk;
                phase = 5U;
            }
            else if (scenario == READ_EARLY_STOP)
            {
                I2C5->ISR = I2C_ISR_STOPF_Msk;
                phase = 6U;
            }
            else if (scenario == RXNE_TIMEOUT)
            {
                phase = 7U;
            }
            else
            {
                I2C5->RXDR = response;
                I2C5->ISR |= I2C_ISR_RXNE_Msk;
                phase = 3U;
                if ((scenario == SIMULTANEOUS_RX_STOP) || (scenario == NACK_WITH_DATA))
                {
                    I2C5->ISR = I2C_ISR_RXNE_Msk | I2C_ISR_STOPF_Msk;
                    if (scenario == NACK_WITH_DATA)
                    {
                        I2C5->ISR |= I2C_ISR_NACKF_Msk;
                    }
                    phase = 6U;
                }
            }
            break;

        case 3:
            I2C5->ISR = (scenario == STOP_TIMEOUT) ? I2C_ISR_BUSY_Msk : I2C_ISR_STOPF_Msk;
            phase = (scenario == STOP_TIMEOUT) ? 7U : 6U;
            break;

        case 5:
            CHECK((I2C5->CR2 & I2C_CR2_STOP_Msk) == 0U);
            I2C5->ISR = I2C_ISR_NACKF_Msk | I2C_ISR_STOPF_Msk;
            phase = 6U;
            break;

        default:
            break;
    }
    return tick++;
}

static void setup(enum scenario selected, uint32_t initial_tick,
                  uint8_t addr, uint8_t reg, uint8_t sample)
{
    memset(I2C5, 0, sizeof(*I2C5));
    I2C5->CR1 = I2C_CR1_PE_Msk;
    I2C5->CR2 = I2C_CR2_ADD10_Msk | I2C_CR2_RD_WRN_Msk | I2C_CR2_SADD_Msk |
                I2C_CR2_NBYTES_Msk | I2C_CR2_RELOAD_Msk | I2C_CR2_AUTOEND_Msk;
    I2C5->TXDR = 0xA5U;
    if ((selected == BUSY_TIMEOUT) || (selected == BUSY_THEN_FREE))
    {
        I2C5->ISR = I2C_ISR_BUSY_Msk;
    }
    scenario = selected;
    phase = polls = writes = restarts = 0U;
    tick = initial_tick;
    expected_addr = addr;
    expected_reg = reg;
    response = sample;
}

static void run_case(enum scenario selected, int expected_result, uint32_t initial_tick,
                     uint8_t addr, uint8_t reg, uint8_t sample, unsigned expected_restarts)
{
    setup(selected, initial_tick, addr, reg, sample);
    uint8_t output = 0x5AU;
    const uint32_t budget = (selected == BUSY_THEN_FREE) ? 30U : 12U;
    const int result = I2C_ReadRegisterByte(I2C5, addr, reg, &output, budget);
    CHECK(result == expected_result);
    CHECK(output == ((expected_result == 0) ? sample : 0x5AU));
    CHECK(restarts == expected_restarts);
    if (expected_result == -2)
    {
        CHECK((tick - initial_tick) == budget + 1U); /* one shared budget */
        CHECK(((I2C5->CR2 & I2C_CR2_STOP_Msk) != 0U) == (selected != BUSY_TIMEOUT));
    }
    else
    {
        CHECK(phase == 6U);
        CHECK(I2C5->ICR == (I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk));
        CHECK((I2C5->CR2 & I2C_CR2_STOP_Msk) == 0U);
    }
    CHECK(writes == ((selected == ADDRESS_NACK || selected == WRITE_EARLY_STOP ||
                      selected == TXIS_TIMEOUT || selected == BUSY_TIMEOUT) ? 0U : 1U));
}

int main(void)
{
    void *const region = VirtualAlloc((void *)(uintptr_t)0x40010000U, 0x10000U,
                                     MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (region == NULL)
    {
        fprintf(stderr, "VirtualAlloc error %lu\n", GetLastError());
        return 2;
    }

    setup(NORMAL, 0U, 0x4AU, 1U, 0xD9U);
    i2c_t before;
    memcpy(&before, I2C5, sizeof(before));
    uint8_t output = 0x5AU;
    CHECK(I2C_ReadRegisterByte(NULL, 0x4AU, 1U, &output, 12U) == -1);
    CHECK(I2C_ReadRegisterByte(&before, 0x4AU, 1U, &output, 12U) == -1);
    CHECK(I2C_ReadRegisterByte(I2C5, 0x80U, 1U, &output, 12U) == -1);
    CHECK(I2C_ReadRegisterByte(I2C5, 0x4AU, 1U, NULL, 12U) == -1);
    CHECK(polls == 0U && output == 0x5AU);
    CHECK(memcmp(&before, I2C5, sizeof(before)) == 0);
    CHECK(I2C_ReadRegisterByte(I2C5, 0x4AU, 1U, &output, 0U) == -2);
    CHECK(memcmp(&before, I2C5, sizeof(before)) == 0);

    run_case(NORMAL, 0, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(SIMULTANEOUS_RX_STOP, 0, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(NORMAL, 0, UINT32_MAX - 3U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(NORMAL, 0, 0U, 0x7FU, 0xFFU, 0x00U, 1U);
    run_case(NORMAL, 0, 0U, 0x4AU, 0x00U, 0xFFU, 1U);
    run_case(ADDRESS_NACK, -3, 0U, 0x4AU, 1U, 0xD9U, 0U);
    run_case(REGISTER_NACK, -3, 0U, 0x4AU, 1U, 0xD9U, 0U);
    run_case(READ_NACK, -3, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(WRITE_EARLY_STOP, -4, 0U, 0x4AU, 1U, 0xD9U, 0U);
    run_case(READ_EARLY_STOP, -4, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(BUSY_TIMEOUT, -2, 0U, 0x4AU, 1U, 0xD9U, 0U);
    run_case(TXIS_TIMEOUT, -2, 0U, 0x4AU, 1U, 0xD9U, 0U);
    run_case(TC_TIMEOUT, -2, 0U, 0x4AU, 1U, 0xD9U, 0U);
    run_case(RXNE_TIMEOUT, -2, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(STOP_TIMEOUT, -2, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(STOP_TIMEOUT, -2, UINT32_MAX - 3U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(BUSY_THEN_FREE, 0, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(NACK_WITH_DATA, -3, 0U, 0x4AU, 1U, 0xD9U, 1U);
    run_case(NACK_WITH_TC, -3, 0U, 0x4AU, 1U, 0xD9U, 0U);

    CHECK(VirtualFree(region, 0U, MEM_RELEASE) != 0);
    printf("%u checks, %u failures\n", checks, failures);
    return failures != 0U;
}
