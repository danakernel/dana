/* kern/clock.h - System clock interface
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KERN_CLOCK_H
#define KERN_CLOCK_H

#include <stdint.h>

void clock_init(void);
uint64_t clock_get_ticks(void);
uint64_t clock_get_uptime_ms(void);
void clock_delay_ms(uint64_t ms);

#endif /* KERN_CLOCK_H */
