/* hal/x86_64/cpu.c - Per-CPU data management
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <hal/x86_64/cpu.h>
#include <hal/x86_64/msr.h>
#include <hal/x86_64/io.h>
#include <kern/kern_types.h>
#include <kern/sched.h>
#include <kern/thread.h>
#include <vm/pmm.h>
#include <vm/zalloc.h>
#include <hal/pmap.h>

static struct cpu_data cpu_data_store[MAX_CPUS];
static uint32_t next_cpu_number = 0;

struct cpu_data *cpu_data_ptr;

static inline void write_fs_base(uint64_t base)
{
    write_msr(MSR_FS_BASE, base);
}

void cpu_init(void)
{
    for (int i = 0; i < MAX_CPUS; i++) {
        cpu_data_store[i].cpu_current_thread = THREAD_NULL;
        cpu_data_store[i].cpu_idle_thread = THREAD_NULL;
        cpu_data_store[i].cpu_id = 0;
        cpu_data_store[i].cpu_number = 0;
        cpu_data_store[i].cpu_stack_top = 0;
        cpu_data_store[i].cpu_tss_rsp0 = 0;
        cpu_data_store[i].cpu_active = 0;
        cpu_data_store[i].cpu_ticks = 0;
    }
}

void cpu_init_self(uint32_t cpu_id)
{
    uint32_t cpu_num = next_cpu_number++;
    struct cpu_data *cpu = &cpu_data_store[cpu_num];

    cpu->cpu_id = cpu_id;
    cpu->cpu_number = cpu_num;
    cpu->cpu_active = 1;

    cpu_data_ptr = cpu;
    write_fs_base((uint64_t)cpu);
}

struct cpu_data *cpu_get(void)
{
    return cpu_data_ptr;
}

uint32_t cpu_number(void)
{
    struct cpu_data *cpu = cpu_get();
    return cpu ? cpu->cpu_number : 0;
}

struct thread *cpu_current_thread(void)
{
    struct cpu_data *cpu = cpu_get();
    return cpu ? cpu->cpu_current_thread : THREAD_NULL;
}

void cpu_set_current_thread(struct thread *thread)
{
    struct cpu_data *cpu = cpu_get();
    if (cpu)
        cpu->cpu_current_thread = thread;
}

struct thread *cpu_idle_thread(void)
{
    struct cpu_data *cpu = cpu_get();
    return cpu ? cpu->cpu_idle_thread : THREAD_NULL;
}

void cpu_set_idle_thread(struct thread *thread)
{
    struct cpu_data *cpu = cpu_get();
    if (cpu)
        cpu->cpu_idle_thread = thread;
}

int cpu_is_boot_cpu(void)
{
    struct cpu_data *cpu = cpu_get();
    return cpu && cpu->cpu_number == 0;
}

void cpu_halt(void)
{
    __asm__ volatile ("cli; hlt" ::: "memory");
}
