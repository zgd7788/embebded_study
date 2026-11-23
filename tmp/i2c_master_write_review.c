/* Host event checks for the real I2C_MasterWrite implementation.
 * RAM and timebase callbacks model selected flags, not electrical timing. */
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"
#include "timebase.h"

enum mode { NORMAL, ADDRESS_NACK, DATA_NACK, EARLY_STOP, BUSY_TIMEOUT,
            TXIS_TIMEOUT, STOP_TIMEOUT, NACK_WITH_TXIS, BUSY_THEN_FREE };
static enum mode mode;
static uint32_t tick;
static unsigned polls, checks, failures;
static size_t delivered, length, nack_after;
static int started, finished;
static const uint8_t *payload;

#define CHECK(c) do { ++checks; if (!(c)) { ++failures; \
    printf("FAIL mode=%d delivered=%u line=%d: %s\n", \
           (int)mode, (unsigned)delivered, __LINE__, #c); } } while (0)

uint32_t Timebase_GetTicks(gtim_t *timer)
{
    CHECK(timer == TIM2);
    if (++polls > 1200U)
    {
        fprintf(stderr, "poll limit exceeded\n");
        exit(2);
    }
    if (mode == BUSY_TIMEOUT)
    {
        return tick++;
    }
    if ((mode == BUSY_THEN_FREE) && (polls < 4U))
    {
        I2C5->ISR = I2C_ISR_BUSY_Msk;
        return tick++;
    }
    if ((mode == BUSY_THEN_FREE) && (polls == 4U))
    {
        I2C5->ISR = 0U;
    }

    if (!started && ((I2C5->CR2 & I2C_CR2_START_Msk) != 0U))
    {
        const uint32_t cr2 = I2C5->CR2;
        CHECK((cr2 & I2C_CR2_SADD_Msk) == (0x4AU << 1U));
        CHECK((cr2 & I2C_CR2_NBYTES_Msk) == ((uint32_t)length << I2C_CR2_NBYTES_Pos));
        CHECK((cr2 & I2C_CR2_AUTOEND_Msk) != 0U);
        CHECK((cr2 & (I2C_CR2_RD_WRN_Msk | I2C_CR2_ADD10_Msk |
                      I2C_CR2_RELOAD_Msk | I2C_CR2_STOP_Msk)) == 0U);
        CHECK(I2C5->ICR == (I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk));
        I2C5->CR2 &= ~I2C_CR2_START_Msk;
        started = 1;
        I2C5->ISR = I2C_ISR_BUSY_Msk;
        if (mode == ADDRESS_NACK || mode == NACK_WITH_TXIS)
        {
            I2C5->ISR |= I2C_ISR_NACKF_Msk;
            if (mode == NACK_WITH_TXIS)
            {
                I2C5->ISR |= I2C_ISR_TXIS_Msk;
            }
        }
        else if (mode != TXIS_TIMEOUT)
        {
            I2C5->ISR |= I2C_ISR_TXIS_Msk;
        }
    }
    else if ((I2C5->ISR & I2C_ISR_NACKF_Msk) != 0U)
    {
        if (mode == NACK_WITH_TXIS)
        {
            CHECK(I2C5->TXDR == 0xA5U); /* NACK suppresses further submission */
        }
        I2C5->ISR = I2C_ISR_STOPF_Msk | I2C_ISR_NACKF_Msk;
        finished = 1;
    }
    else if ((I2C5->ISR & I2C_ISR_TXIS_Msk) != 0U)
    {
        CHECK(delivered < length);
        CHECK(I2C5->TXDR == payload[delivered]);
        ++delivered;
        CHECK((I2C5->CR2 & I2C_CR2_STOP_Msk) == 0U);
        if ((mode == DATA_NACK) && (delivered == nack_after))
        {
            I2C5->ISR = I2C_ISR_BUSY_Msk | I2C_ISR_NACKF_Msk;
        }
        else if ((mode == EARLY_STOP) && (delivered == 1U))
        {
            I2C5->ISR = I2C_ISR_STOPF_Msk;
            finished = 1;
        }
        else
        {
            I2C5->ISR = I2C_ISR_BUSY_Msk;
            if (delivered < length)
            {
                I2C5->ISR |= I2C_ISR_TXIS_Msk;
            }
        }
    }
    else if (started && delivered == length && mode != STOP_TIMEOUT)
    {
        I2C5->ISR = I2C_ISR_STOPF_Msk;
        finished = 1;
    }
    return tick++;
}

static void setup(enum mode selected, uint32_t initial_tick,
                  const uint8_t *data, size_t count, size_t nack_index)
{
    memset(I2C5, 0, sizeof(*I2C5));
    I2C5->CR1 = I2C_CR1_PE_Msk;
    I2C5->CR2 = I2C_CR2_SADD_Msk | I2C_CR2_RD_WRN_Msk | I2C_CR2_ADD10_Msk |
                I2C_CR2_NBYTES_Msk | I2C_CR2_RELOAD_Msk;
    I2C5->TXDR = 0xA5U;
    if (selected == BUSY_TIMEOUT || selected == BUSY_THEN_FREE)
    {
        I2C5->ISR = I2C_ISR_BUSY_Msk;
    }
    mode = selected;
    tick = initial_tick;
    polls = 0U;
    delivered = 0U;
    started = finished = 0;
    payload = data;
    length = count;
    nack_after = nack_index;
}

static void run_case(enum mode selected, uint32_t initial_tick,
                     const uint8_t *data, size_t count, size_t nack_index,
                     int expected_result, size_t expected_delivered)
{
    setup(selected, initial_tick, data, count, nack_index);
    const uint32_t budget = (expected_result == -2) ? 12U : 1000U;
    uint8_t copy[255];
    memcpy(copy, data, count);
    const uint32_t old_cr2 = I2C5->CR2;
    CHECK(I2C_MasterWrite(I2C5, 0x4AU, data, count, budget) == expected_result);
    CHECK(delivered == expected_delivered);
    CHECK(memcmp(copy, data, count) == 0);
    if (expected_result == -2)
    {
        CHECK((tick - initial_tick) == budget + 1U);
        CHECK(((I2C5->CR2 & I2C_CR2_STOP_Msk) != 0U) == (selected != BUSY_TIMEOUT));
        if (selected == BUSY_TIMEOUT)
        {
            CHECK(I2C5->CR2 == old_cr2 && I2C5->ICR == 0U);
        }
    }
    else
    {
        CHECK(finished);
        CHECK(I2C5->ICR == (I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk));
        CHECK((I2C5->CR2 & I2C_CR2_STOP_Msk) == 0U);
    }
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
    uint8_t buffer[255];
    for (size_t i = 0U; i < sizeof(buffer); ++i)
    {
        buffer[i] = (uint8_t)(i ^ 0x5AU);
    }
    setup(NORMAL, 0U, buffer, 2U, 0U);
    i2c_t before;
    memcpy(&before, I2C5, sizeof(before));
    CHECK(I2C_MasterWrite(NULL, 0x4AU, buffer, 2U, 12U) == -1);
    CHECK(I2C_MasterWrite(&before, 0x4AU, buffer, 2U, 12U) == -1);
    CHECK(I2C_MasterWrite(I2C5, 0x80U, buffer, 2U, 12U) == -1);
    CHECK(I2C_MasterWrite(I2C5, 0x4AU, NULL, 2U, 12U) == -1);
    CHECK(I2C_MasterWrite(I2C5, 0x4AU, buffer, 0U, 12U) == -1);
    CHECK(I2C_MasterWrite(I2C5, 0x4AU, buffer, 256U, 12U) == -1);
    CHECK(I2C_MasterWrite(I2C5, 0x4AU, buffer, SIZE_MAX, 12U) == -1);
    CHECK(polls == 0U && memcmp(&before, I2C5, sizeof(before)) == 0);
    CHECK(I2C_MasterWrite(I2C5, 0x4AU, buffer, 2U, 0U) == -2);
    CHECK(memcmp(&before, I2C5, sizeof(before)) == 0);

    run_case(NORMAL, 0U, buffer, 1U, 0U, 0, 1U);
    run_case(NORMAL, 0U, buffer, 2U, 0U, 0, 2U);
    run_case(NORMAL, 0U, buffer, 255U, 0U, 0, 255U);
    run_case(NORMAL, UINT32_MAX - 3U, buffer, 3U, 0U, 0, 3U);
    run_case(ADDRESS_NACK, 0U, buffer, 3U, 0U, -3, 0U);
    run_case(DATA_NACK, 0U, buffer, 3U, 1U, -3, 1U);
    run_case(DATA_NACK, 0U, buffer, 3U, 3U, -3, 3U);
    run_case(NACK_WITH_TXIS, 0U, buffer, 3U, 0U, -3, 0U);
    run_case(EARLY_STOP, 0U, buffer, 3U, 0U, -4, 1U);
    run_case(BUSY_TIMEOUT, 0U, buffer, 3U, 0U, -2, 0U);
    run_case(TXIS_TIMEOUT, 0U, buffer, 3U, 0U, -2, 0U);
    run_case(STOP_TIMEOUT, 0U, buffer, 3U, 0U, -2, 3U);
    run_case(STOP_TIMEOUT, UINT32_MAX - 3U, buffer, 3U, 0U, -2, 3U);
    run_case(BUSY_THEN_FREE, 0U, buffer, 3U, 0U, 0, 3U);

    CHECK(VirtualFree(region, 0U, MEM_RELEASE) != 0);
    printf("%u checks, %u failures\n", checks, failures);
    return failures != 0U;
}
