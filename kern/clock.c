/* kern/clock.c - System clock implementation
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/clock.h>
#include <hal/x86_64/pit.h>

#define CLOCK_HZ 100

void clock_init(void)
{
    pit_init(CLOCK_HZ);
}

uint64_t clock_get_ticks(void)
{
    return pit_get_ticks();
}

uint64_t clock_get_uptime_ms(void)
{
    return pit_get_uptime_ms();
}

void clock_delay_ms(uint64_t ms)
{
    uint64_t start = clock_get_uptime_ms();
    while (clock_get_uptime_ms() - start < ms) {
        __asm__ volatile ("pause" ::: "memory");
    }
}
