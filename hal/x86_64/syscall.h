/* hal/x86_64/syscall.h - x86_64 syscall interface
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef HAL_X86_64_SYSCALL_H
#define HAL_X86_64_SYSCALL_H

#include <stdint.h>

void syscall_arch_init(void);
void syscall_entry(void);

#endif /* HAL_X86_64_SYSCALL_H */
