/* hal/x86_64/pit.h - Intel 8254 PIT driver
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef HAL_X86_64_PIT_H
#define HAL_X86_64_PIT_H

#include <stdint.h>

#define PIT_FREQUENCY   1193182
#define PIT_CHANNEL0    0x40
#define PIT_CHANNEL1    0x41
#define PIT_CHANNEL2    0x42
#define PIT_CONTROL     0x43

void pit_init(uint32_t frequency);
void pit_set_frequency(uint32_t frequency);
uint32_t pit_get_frequency(void);
uint64_t pit_get_ticks(void);
uint64_t pit_get_uptime_ms(void);
void pit_increment_tick(void);

#endif /* HAL_X86_64_PIT_H */
