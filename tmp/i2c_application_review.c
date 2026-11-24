/* Execute the real application with platform/I2C stubs. Checks restoration,
 * burst-read comparisons and diagnostics, not physical hardware. */
#include <stdio.h>
#include <string.h>
#define main i2c_application_main
#include "../project/stage_4/261122/user/main.c"
#undef main

enum fault { NONE, ID_READ_FAIL, WRONG_ID, ORIGINAL_READ_FAIL,
             PARTIAL_WRITE_FAIL, TEST_READ_FAIL, TEST_MISMATCH,
             RESTORE_WRITE_FAIL, RESTORE_READ_FAIL, RESTORE_MISMATCH,
             REFERENCE_READ_FAIL, MAP_WRITE_FAIL, BURST_READ_FAIL,
             BURST_MISMATCH_FIRST, BURST_MISMATCH_MIDDLE, BURST_MISMATCH_LAST,
             BURST_REPEATED_REGISTER };
static enum fault fault;
static uint8_t saved, reg_value;
static unsigned reads, writes, checks, failures;
static unsigned reference_reads, map_writes, burst_reads;
static uint8_t reference_values[3];
static uint32_t ticks;
static char output[4096];
static size_t output_length;

#define CHECK(c) do { ++checks; if (!(c)) { ++failures; \
    printf("FAIL fault=%d line=%d: %s\n", (int)fault, __LINE__, #c); } } while (0)

int RCC_GPIO_EnableClock(volatile gpio_t *port) { (void)port; return 0; }
void RCC_TIM2_EnableClock(void) { }
void RCC_UART4_EnableClock(void) { }
void RCC_I2C5_EnableClock(void) { }
int RCC_HSI_EnableAndWaitReady(void) { return 0; }
int RCC_UART4_SelectClock(uart4_clock_source_e source) { (void)source; return 0; }
int RCC_UART4_GetKernelClockHz(uint32_t *hz) { *hz = 64000000U; return 0; }
int UART4_DeInit(void) { return 0; }
int UART4_Init(const uart_init_t *config, uint32_t hz) { (void)config; (void)hz; return 0; }
void HAL_GPIO_Init(volatile gpio_t *port, const gpioinit_t *config) { (void)port; (void)config; }
uint8_t HAL_GPIO_Write(volatile gpio_t *port, uint16_t pin, gpio_state_e value)
{ (void)port; (void)pin; (void)value; return 0U; }
int GTIM_BaseInit(gtim_t *tim, uint16_t psc, uint32_t arr)
{ (void)tim; (void)psc; (void)arr; return 0; }
void GTIMx_Start(gtim_t *tim) { (void)tim; }
uint32_t Timebase_GetTicks(gtim_t *tim) { (void)tim; return ticks++; }
int I2C_Init(i2c_t *i2c, uint32_t timing) { (void)i2c; (void)timing; return 0; }
int HAL_UART4_PutCharTimeout(uint8_t ch, uint32_t timeout)
{
    (void)timeout;
    if (output_length + 1U >= sizeof(output))
    {
        return -2;
    }
    output[output_length++] = (char)ch;
    output[output_length] = '\0';
    return 0;
}
int HAL_UART4_WaitTxComplete(uint32_t timeout) { (void)timeout; return 0; }

int I2C_ReadRegisterByte(i2c_t *i2c, uint8_t addr, uint8_t reg,
                         uint8_t *data, uint32_t timeout)
{
    CHECK(i2c == I2C5 && addr == 0x4AU && data != NULL && timeout != 0U);
    ++reads;
    if (reg >= 0x01U && reg <= 0x03U && writes == 2U)
    {
        CHECK(reg == 0x01U + reference_reads);
        ++reference_reads;
        if (fault == REFERENCE_READ_FAIL && reg == 0x02U) return -2;
        *data = reference_values[reg - 1U];
        return 0;
    }
    if (reg == 0x01U)
    {
        CHECK(reads == 1U && writes == 0U);
        if (fault == ID_READ_FAIL) return -3;
        *data = (fault == WRONG_ID) ? 0x00U : 0xD9U;
        return 0;
    }
    CHECK(reg == 0x0CU);
    if (writes == 0U && fault == ORIGINAL_READ_FAIL) return -2;
    if (writes == 1U && fault == TEST_READ_FAIL) return -3;
    if (writes == 2U && fault == RESTORE_READ_FAIL) return -2;
    *data = reg_value;
    if ((writes == 1U && fault == TEST_MISMATCH) ||
        (writes == 2U && fault == RESTORE_MISMATCH))
    {
        *data ^= 1U;
    }
    return 0;
}

int I2C_MasterWrite(i2c_t *i2c, uint8_t addr, const uint8_t *data,
                    size_t count, uint32_t timeout)
{
    CHECK(i2c == I2C5 && addr == 0x4AU && timeout != 0U);
    if (count == 1U)
    {
        ++map_writes;
        CHECK(writes == 2U && reference_reads == 3U);
        CHECK(data[0] == 0x81U && map_writes == 1U);
        return (fault == MAP_WRITE_FAIL) ? -3 : 0;
    }
    CHECK(count == 2U);
    CHECK(data[0] == 0x0CU); /* never writes ID, power, or reserved registers */
    ++writes;
    CHECK(writes <= 2U);
    if (writes == 1U)
    {
        CHECK(data[1] != saved);
        CHECK(data[1] == ((saved == 0xFFU) ? 0xFEU : 0xFFU));
    }
    else
    {
        CHECK(data[1] == saved);
        if (fault == RESTORE_WRITE_FAIL) return -2;
    }
    reg_value = data[1];
    if (writes == 1U && fault == PARTIAL_WRITE_FAIL) return -3;
    return 0;
}

int I2C_MasterRead(i2c_t *i2c, uint8_t addr, uint8_t *data,
                   size_t count, uint32_t timeout)
{
    CHECK(i2c == I2C5 && addr == 0x4AU && count == 3U && timeout != 0U);
    CHECK(reference_reads == 3U && map_writes == 1U);
    ++burst_reads;
    CHECK(burst_reads == 1U);
    data[0] = reference_values[0];
    if (fault == BURST_READ_FAIL) return -2; /* partial data must not be used */
    memcpy(data, reference_values, count);
    if (fault >= BURST_MISMATCH_FIRST && fault <= BURST_MISMATCH_LAST)
    {
        data[(unsigned)fault - BURST_MISMATCH_FIRST] ^= 1U;
    }
    if (fault == BURST_REPEATED_REGISTER)
    {
        data[1] = data[2] = data[0];
    }
    return 0;
}

static void run_case(enum fault selected, uint8_t original)
{
    fault = selected;
    saved = reg_value = original;
    reads = writes = 0U;
    reference_reads = map_writes = burst_reads = 0U;
    reference_values[0] = 0xD9U;
    reference_values[1] = (original == 0x55U) ? 0x35U : 0x00U;
    reference_values[2] = (original == 0x55U) ? 0x97U : 0xAEU;
    ticks = 0U;
    output_length = 0U;
    output[0] = '\0';
    CHECK(i2c_application_main() == ((selected == NONE) ? 0 : 1));
    const int no_write = selected == ID_READ_FAIL || selected == WRONG_ID ||
                         selected == ORIGINAL_READ_FAIL;
    CHECK(writes == (no_write ? 0U : 2U));
    CHECK(reg_value == ((selected == RESTORE_WRITE_FAIL) ?
                        ((original == 0xFFU) ? 0xFEU : 0xFFU) : original));
    const int write_passed = selected == NONE || selected >= REFERENCE_READ_FAIL;
    CHECK((strstr(output, "I2C multi-byte write PASS\r\n") != NULL) == write_passed);
    CHECK((strstr(output, "I2C multi-byte read PASS\r\n") != NULL) == (selected == NONE));
    const int reaches_burst = write_passed && selected != REFERENCE_READ_FAIL && selected != MAP_WRITE_FAIL;
    CHECK(burst_reads == (reaches_burst ? 1U : 0U));
    CHECK(map_writes == ((write_passed && selected != REFERENCE_READ_FAIL) ? 1U : 0U));
    CHECK((strstr(output, "single=") != NULL) == (reaches_burst && selected != BURST_READ_FAIL));
    CHECK((strstr(output, "restore PASS\r\n") != NULL) ==
          (!no_write && selected != RESTORE_WRITE_FAIL &&
           selected != RESTORE_READ_FAIL && selected != RESTORE_MISMATCH));
    if (selected == NONE && original == 0xFFU)
    {
        CHECK(strstr(output, "REG0x0C test=0xFE\r\n") != NULL);
    }
    if (selected == NONE && original == 0U)
    {
        CHECK(strstr(output, "REG0x0C readback=0xFF\r\n") != NULL);
        CHECK(strstr(output, "REG0x0C restored=0x00\r\n") != NULL);
    }
    if (selected == PARTIAL_WRITE_FAIL || selected == TEST_READ_FAIL || selected == TEST_MISMATCH)
    {
        CHECK(strstr(output, "I2C multi-byte write FAIL\r\n") != NULL);
    }
    if (selected >= BURST_MISMATCH_FIRST)
    {
        CHECK(strstr(output, "I2C multi-byte read FAIL\r\n") != NULL);
    }
    if (selected == NONE)
    {
        CHECK(strstr(output, "REG0x01 single=0xD9 burst=0xD9\r\n") != NULL);
        CHECK(strstr(output, original == 0x55U ? "REG0x02 single=0x35 burst=0x35\r\n" :
                     "REG0x02 single=0x00 burst=0x00\r\n") != NULL);
        CHECK(strstr(output, original == 0x55U ? "REG0x03 single=0x97 burst=0x97\r\n" :
                     "REG0x03 single=0xAE burst=0xAE\r\n") != NULL);
    }
}

int main(void)
{
    run_case(NONE, 0x00U);
    run_case(NONE, 0xFFU);
    run_case(NONE, 0x55U);
    for (int selected = ID_READ_FAIL; selected <= BURST_REPEATED_REGISTER; ++selected)
    {
        run_case((enum fault)selected, 0x00U);
    }
    printf("%u checks, %u failures\n", checks, failures);
    return failures != 0U;
}
