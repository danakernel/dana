/* hal/x86_64/syscall.c - x86_64 syscall setup
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <hal/x86_64/syscall.h>
#include <hal/x86_64/msr.h>
#include <hal/x86_64/io.h>

#define KERNEL_CS   0x08
#define USER_CS     0x1B
#define KERNEL_SS   0x10
#define USER_SS     0x23

extern void syscall_entry(void);

void syscall_arch_init(void)
{
    uint64_t lstar = (uint64_t)syscall_entry;
    uint64_t star = ((uint64_t)(USER_CS) << 48) | ((uint64_t)(KERNEL_CS) << 32);
    uint64_t fmask = 0x200600;

    write_msr(MSR_LSTAR, lstar);
    write_msr(MSR_STAR, star);
    write_msr(MSR_FMASK, fmask);

    uint64_t efer = read_msr(MSR_EFER);
    efer |= (1 << 0);
    write_msr(MSR_EFER, efer);
}
