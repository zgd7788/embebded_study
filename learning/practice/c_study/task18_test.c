#include "task18_gpio_sim.h"
#include <stdio.h>

static int check(int condition, const char *name)
{
    if (!condition)
    {
        printf("FAIL: %s\n", name);
        return 0;
    }

    printf("PASS: %s\n", name);
    return 1;
}

int main(void)
{
    const uint32_t enable_mask = 0x1u;
    const uint32_t mode_mask = 0x3u << 1;
    const uint32_t base_control = 0xF8u;
    gpio_regs_t regs = {
        .control = base_control,
        .status = 1u
    };
    gpio_t gpio = {0};
    gpio_t empty_gpio = {0};
    int ready = -1;
    int failures = 0;

    failures += !check(gpio_init(NULL, &regs) == -1,
                       "gpio_init NULL context");
    failures += !check(gpio_init(&gpio, NULL) == -1,
                       "gpio_init NULL registers");
    failures += !check(gpio_init(&gpio, &regs) == 0 && gpio.regs == &regs,
                       "gpio_init success and binding");

    failures += !check(gpio_set_enable(&empty_gpio, 1) == -1,
                       "set_enable unbound context");
    failures += !check(gpio_set_enable(NULL, 1) == -1,
                       "set_enable NULL context");
    failures += !check(gpio_set_enable(&gpio, 2) == -1,
                       "set_enable invalid value");

    regs.control = base_control;
    failures += !check(gpio_set_enable(&gpio, 1) == 0 &&
                       regs.control == (base_control | enable_mask),
                       "set_enable 1 preserves other bits");
    failures += !check(gpio_set_enable(&gpio, 0) == 0 &&
                       regs.control == base_control,
                       "set_enable 0 preserves other bits");

    failures += !check(gpio_set_mode(&empty_gpio, 1) == -1,
                       "set_mode unbound context");
    failures += !check(gpio_set_mode(NULL, 1) == -1,
                       "set_mode NULL context");
    failures += !check(gpio_set_mode(&gpio, 4) == -1,
                       "set_mode invalid value");

    regs.control = base_control;
    for (uint32_t mode = 0; mode <= 3; mode++)
    {
        uint32_t expected = (regs.control & ~mode_mask) | (mode << 1);
        char name[32];
        (void)snprintf(name, sizeof(name), "set_mode value %u", mode);
        failures += !check(gpio_set_mode(&gpio, mode) == 0 &&
                           regs.control == expected,
                           name);
    }

    failures += !check(gpio_get_ready(&empty_gpio, &ready) == -1,
                       "get_ready unbound context");
    failures += !check(gpio_get_ready(NULL, &ready) == -1,
                       "get_ready NULL context");
    failures += !check(gpio_get_ready(&gpio, NULL) == -1,
                       "get_ready NULL output");

    regs.status = 1u;
    ready = -1;
    failures += !check(gpio_get_ready(&gpio, &ready) == 0 && ready == 1,
                       "get_ready returns 1");
    regs.status = 0u;
    ready = -1;
    failures += !check(gpio_get_ready(&gpio, &ready) == 0 && ready == 0,
                       "get_ready returns 0");

    printf("%s\n", failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED");
    return failures == 0 ? 0 : 1;
}
