/* Host event model for the actual I2C_MasterRead driver. Ordinary RAM stands
 * in for MMIO; timebase callbacks advance a scripted slave response.
 * It checks bounds and event handling, not bus timing or electrical ACKs. */
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"
#include "timebase.h"

enum mode { NORMAL, LAST_RX_STOP, ADDRESS_NACK, EARLY_STOP, EARLY_RX_STOP,
            BUSY_TIMEOUT, RXNE_TIMEOUT, PARTIAL_TIMEOUT, STOP_TIMEOUT,
            BUSY_THEN_FREE, NACK_WITH_RX_STOP, EXTRA_RX_STOP };
static enum mode mode;
static uint32_t tick;
static unsigned polls, checks, failures;
static size_t published, length;
static int started, finished;
static uint8_t *destination;
static uint8_t response[255];

#define CHECK(c) do { ++checks; if (!(c)) { ++failures; \
    printf("FAIL mode=%d published=%u line=%d: %s\n", \
           (int)mode, (unsigned)published, __LINE__, #c); } } while (0)

static void publish_next(void)
{
    CHECK(published < length);
    I2C5->RXDR = response[published++];
    I2C5->ISR = I2C_ISR_BUSY_Msk | I2C_ISR_RXNE_Msk;
    if ((mode == LAST_RX_STOP && published == length) ||
        (mode == EARLY_RX_STOP && published == 1U) || mode == NACK_WITH_RX_STOP)
    {
        I2C5->ISR = I2C_ISR_RXNE_Msk | I2C_ISR_STOPF_Msk;
        if (mode == NACK_WITH_RX_STOP) I2C5->ISR |= I2C_ISR_NACKF_Msk;
        finished = 1;
    }
}

uint32_t Timebase_GetTicks(gtim_t *timer)
{
    CHECK(timer == TIM2);
    if (++polls > 1200U)
    {
        fprintf(stderr, "poll limit exceeded\n");
        exit(2);
    }
    if (mode == BUSY_TIMEOUT) return tick++;
    if (mode == BUSY_THEN_FREE && polls < 4U)
    {
        I2C5->ISR = I2C_ISR_BUSY_Msk;
        return tick++;
    }
    if (mode == BUSY_THEN_FREE && polls == 4U) I2C5->ISR = 0U;

    if (!started && (I2C5->CR2 & I2C_CR2_START_Msk) != 0U)
    {
        const uint32_t cr2 = I2C5->CR2;
        CHECK((cr2 & I2C_CR2_SADD_Msk) == (0x4AU << 1U));
        CHECK((cr2 & I2C_CR2_NBYTES_Msk) == ((uint32_t)length << I2C_CR2_NBYTES_Pos));
        CHECK((cr2 & I2C_CR2_RD_WRN_Msk) != 0U);
        CHECK((cr2 & I2C_CR2_AUTOEND_Msk) != 0U);
        CHECK((cr2 & (I2C_CR2_ADD10_Msk | I2C_CR2_RELOAD_Msk | I2C_CR2_STOP_Msk)) == 0U);
        CHECK(I2C5->ICR == (I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk));
        I2C5->CR2 &= ~I2C_CR2_START_Msk;
        started = 1;
        I2C5->ISR = I2C_ISR_BUSY_Msk;
        if (mode == ADDRESS_NACK)
        {
            I2C5->ISR |= I2C_ISR_NACKF_Msk;
        }
        else if (mode == EARLY_STOP)
        {
            I2C5->ISR = I2C_ISR_STOPF_Msk;
            finished = 1;
        }
        else if (mode != RXNE_TIMEOUT)
        {
            publish_next();
        }
    }
    else if ((I2C5->ISR & I2C_ISR_NACKF_Msk) != 0U)
    {
        I2C5->ISR = I2C_ISR_NACKF_Msk | I2C_ISR_STOPF_Msk;
        finished = 1;
    }
    else if ((I2C5->ISR & I2C_ISR_RXNE_Msk) != 0U)
    {
        CHECK(published != 0U);
        CHECK(destination[published - 1U] == response[published - 1U]);
        CHECK((I2C5->CR2 & I2C_CR2_STOP_Msk) == 0U);
        I2C5->ISR = I2C_ISR_BUSY_Msk;
        if (mode == PARTIAL_TIMEOUT)
        {
            /* Leave the partially received prefix in the output buffer. */
        }
        else if (published < length)
        {
            publish_next();
        }
        else if (mode == EXTRA_RX_STOP)
        {
            /* Synthetic extra RXNE verifies that length remains a hard bound. */
            I2C5->RXDR = 0xCCU;
            I2C5->ISR = I2C_ISR_RXNE_Msk | I2C_ISR_STOPF_Msk;
            finished = 1;
        }
    }
    else if (started && published == length && mode != STOP_TIMEOUT)
    {
        I2C5->ISR = I2C_ISR_STOPF_Msk;
        finished = 1;
    }
    return tick++;
}

static void setup(enum mode selected, uint32_t initial_tick, uint8_t *data, size_t count)
{
    memset(I2C5, 0, sizeof(*I2C5));
    I2C5->CR1 = I2C_CR1_PE_Msk;
    I2C5->CR2 = I2C_CR2_SADD_Msk | I2C_CR2_ADD10_Msk |
                I2C_CR2_NBYTES_Msk | I2C_CR2_RELOAD_Msk;
    I2C5->TXDR = 0x13579BDFU;
    if (selected == BUSY_TIMEOUT || selected == BUSY_THEN_FREE)
        I2C5->ISR = I2C_ISR_BUSY_Msk;
    mode = selected;
    tick = initial_tick;
    polls = 0U;
    published = 0U;
    started = finished = 0;
    destination = data;
    length = count;
}

static void run_case(enum mode selected, uint32_t initial_tick, size_t count,
                     int expected_result, size_t expected_received)
{
    uint8_t guarded[257];
    memset(guarded, 0xA5, sizeof(guarded));
    setup(selected, initial_tick, guarded + 1U, count);
    const uint32_t budget = (expected_result == -2) ? 12U : 1000U;
    CHECK(I2C_MasterRead(I2C5, 0x4AU, destination, count, budget) == expected_result);
    CHECK(memcmp(destination, response, expected_received) == 0);
    for (size_t i = expected_received + 1U; i < sizeof(guarded); ++i)
        CHECK(guarded[i] == 0xA5U);
    CHECK(guarded[0] == 0xA5U);
    CHECK(I2C5->TXDR == 0x13579BDFU);
    if (expected_result == -2)
    {
        CHECK((tick - initial_tick) == budget + 1U);
        CHECK(((I2C5->CR2 & I2C_CR2_STOP_Msk) != 0U) == (selected != BUSY_TIMEOUT));
        if (selected == BUSY_TIMEOUT) CHECK(I2C5->ICR == 0U);
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
    for (size_t i = 0U; i < sizeof(response); ++i) response[i] = (uint8_t)(i ^ 0x37U);
    response[0] = 0x00U;
    response[1] = 0xFFU;
    uint8_t data[3] = {0xAAU, 0xBBU, 0xCCU};
    setup(NORMAL, 0U, data, sizeof(data));
    i2c_t before;
    memcpy(&before, I2C5, sizeof(before));
    CHECK(I2C_MasterRead(NULL, 0x4AU, data, 3U, 12U) == -1);
    CHECK(I2C_MasterRead(&before, 0x4AU, data, 3U, 12U) == -1);
    CHECK(I2C_MasterRead(I2C5, 0x80U, data, 3U, 12U) == -1);
    CHECK(I2C_MasterRead(I2C5, 0x4AU, NULL, 3U, 12U) == -1);
    CHECK(I2C_MasterRead(I2C5, 0x4AU, data, 0U, 12U) == -1);
    CHECK(I2C_MasterRead(I2C5, 0x4AU, data, 256U, 12U) == -1);
    CHECK(I2C_MasterRead(I2C5, 0x4AU, data, SIZE_MAX, 12U) == -1);
    CHECK(polls == 0U && memcmp(&before, I2C5, sizeof(before)) == 0);
    CHECK(I2C_MasterRead(I2C5, 0x4AU, data, 3U, 0U) == -2);
    CHECK(memcmp(&before, I2C5, sizeof(before)) == 0);
    CHECK(data[0] == 0xAAU && data[1] == 0xBBU && data[2] == 0xCCU);

    run_case(NORMAL, 0U, 1U, 0, 1U);
    run_case(NORMAL, 0U, 3U, 0, 3U);
    run_case(NORMAL, 0U, 255U, 0, 255U);
    run_case(LAST_RX_STOP, 0U, 1U, 0, 1U);
    run_case(LAST_RX_STOP, 0U, 3U, 0, 3U);
    run_case(LAST_RX_STOP, 0U, 255U, 0, 255U);
    run_case(LAST_RX_STOP, UINT32_MAX - 3U, 3U, 0, 3U);
    run_case(ADDRESS_NACK, 0U, 3U, -3, 0U);
    run_case(EARLY_STOP, 0U, 3U, -4, 0U);
    run_case(EARLY_RX_STOP, 0U, 3U, -4, 1U);
    run_case(NACK_WITH_RX_STOP, 0U, 3U, -3, 1U);
    run_case(BUSY_TIMEOUT, 0U, 3U, -2, 0U);
    run_case(RXNE_TIMEOUT, 0U, 3U, -2, 0U);
    run_case(PARTIAL_TIMEOUT, 0U, 3U, -2, 1U);
    run_case(STOP_TIMEOUT, 0U, 3U, -2, 3U);
    run_case(STOP_TIMEOUT, UINT32_MAX - 3U, 3U, -2, 3U);
    run_case(BUSY_THEN_FREE, 0U, 3U, 0, 3U);
    run_case(EXTRA_RX_STOP, 0U, 3U, 0, 3U);

    CHECK(VirtualFree(region, 0U, MEM_RELEASE) != 0);
    printf("%u checks, %u failures\n", checks, failures);
    return failures != 0U;
}
