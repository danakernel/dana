/* hal/x86_64/cpu.h - Per-CPU data structure for x86_64
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef HAL_X86_64_CPU_H
#define HAL_X86_64_CPU_H

#include <stdint.h>
#include <stddef.h>

struct thread;

#define MAX_CPUS    8

struct cpu_data {
    struct thread   *cpu_current_thread;
    struct thread   *cpu_idle_thread;
    uint32_t        cpu_id;
    uint32_t        cpu_number;
    uint64_t        cpu_stack_top;
    uint64_t        cpu_tss_rsp0;
    volatile int    cpu_active;
    uint64_t        cpu_ticks;
};

void cpu_init(void);
void cpu_init_self(uint32_t cpu_id);
struct cpu_data *cpu_get(void);
uint32_t cpu_number(void);
struct thread *cpu_current_thread(void);
void cpu_set_current_thread(struct thread *thread);
struct thread *cpu_idle_thread(void);
void cpu_set_idle_thread(struct thread *thread);
int cpu_is_boot_cpu(void);
void cpu_halt(void);

extern struct cpu_data *cpu_data_ptr;

#endif /* HAL_X86_64_CPU_H */
