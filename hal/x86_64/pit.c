/* hal/x86_64/pit.c - Intel 8254 PIT driver
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <hal/x86_64/pit.h>
#include <hal/x86_64/io.h>

static uint32_t pit_freq = 0;
static volatile uint64_t pit_ticks = 0;

void pit_init(uint32_t frequency)
{
    uint32_t divisor = PIT_FREQUENCY / frequency;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    cpu_outb(PIT_CONTROL, 0x36);
    cpu_outb(PIT_CHANNEL0, low);
    cpu_outb(PIT_CHANNEL0, high);

    pit_freq = frequency;
    pit_ticks = 0;
}

void pit_set_frequency(uint32_t frequency)
{
    pit_init(frequency);
}

uint32_t pit_get_frequency(void)
{
    return pit_freq;
}

uint64_t pit_get_ticks(void)
{
    return pit_ticks;
}

uint64_t pit_get_uptime_ms(void)
{
    if (pit_freq == 0)
        return 0;
    return (pit_ticks * 1000) / pit_freq;
}

void pit_increment_tick(void)
{
    pit_ticks++;
}
