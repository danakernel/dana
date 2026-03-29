/* kern/thread.c - Mach thread management
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Each kernel thread gets a 4 KB kernel stack allocated from the PMM.
 * thread_create sets up a fake switch frame so that the first call to
 * machine_switch_context restores callee-saved registers and jumps to
 * thread_trampoline, which calls the thread's entry function.
 *
 * Stack layout at thread_create time (addresses decrease downward):
 *
 *   kernel_stack_top - 8   guard word (available for first call frame)
 *   kernel_stack_top - 16  &thread_trampoline  <- ret address
 *   kernel_stack_top - 24  rbp = 0
 *   kernel_stack_top - 32  rbx = 0
 *   kernel_stack_top - 40  r12 = 0
 *   kernel_stack_top - 48  r13 = 0
 *   kernel_stack_top - 56  r14 = arg           <- machine_switch_context pops
 *   kernel_stack_top - 64  r15 = entry fn      <- saved_state.rsp points here
 */

#include <kern/sched.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <vm/zalloc.h>
#include <vm/pmm.h>
#include <hal/pmap.h>
#include <hal/thread.h>
#include <libkern/string.h>

#define KSTACK_SIZE  PMM_PAGE_SIZE

static zone_t   thread_zone;
static uint32_t next_thread_id = 1;

static zone_t get_thread_zone(void)
{
    if (thread_zone == ZONE_NULL)
        thread_zone = zinit("thread", sizeof(struct thread), 0);
    return thread_zone;
}

kern_return_t thread_create(task_t task, void (*entry)(void *), void *arg,
                             thread_t *thread_out)
{
    zone_t z = get_thread_zone();
    if (z == ZONE_NULL)
        return KERN_NO_SPACE;

    thread_t t = zalloc(z);
    if (t == THREAD_NULL)
        return KERN_NO_SPACE;

    uint64_t stack_phys = pmm_alloc_page();
    if (stack_phys == PMM_NULL) {
        zfree(z, t);
        return KERN_NO_SPACE;
    }

    t->thread_id         = next_thread_id++;
    t->ref_count         = 1;
    t->task              = task;
    t->task_next         = THREAD_NULL;
    t->ith_self          = IPC_PORT_NULL;
    t->kernel_stack_phys = stack_phys;
    t->kernel_stack_top  = (uint64_t)(uintptr_t)PHYS_TO_VIRT(stack_phys) + KSTACK_SIZE;
    t->sched.state       = THREAD_STATE_RUNNABLE;
    t->sched.next        = THREAD_NULL;
    t->sched.prev        = THREAD_NULL;
    t->sched.run_time    = 0;

    uint64_t *sp = (uint64_t *)t->kernel_stack_top;
    *--sp = 0;                           /* guard word */
    *--sp = (uint64_t)thread_trampoline; /* ret address for machine_switch_context */
    *--sp = 0;                           /* rbp */
    *--sp = 0;                           /* rbx */
    *--sp = 0;                           /* r12 */
    *--sp = 0;                           /* r13 */
    *--sp = (uint64_t)arg;               /* r14 — argument to entry */
    *--sp = (uint64_t)entry;             /* r15 — entry function */

    t->saved_state.rsp = (uint64_t)sp;

    if (task != TASK_NULL) {
        t->task_next       = task->threads;
        task->threads      = t;
        task->thread_count++;
    }

    sched_enqueue(t);
    *thread_out = t;
    return KERN_SUCCESS;
}

kern_return_t thread_destroy(thread_t t)
{
    if (t == THREAD_NULL)
        return KERN_INVALID_ARGUMENT;
    if (--t->ref_count > 0)
        return KERN_SUCCESS;

    sched_dequeue(t);

    if (t->kernel_stack_phys)
        pmm_free_page(t->kernel_stack_phys);

    zfree(get_thread_zone(), t);
    return KERN_SUCCESS;
}

void thread_switch(thread_t old_thread, thread_t new_thread)
{
    machine_set_rsp0(new_thread->kernel_stack_top);
    machine_switch_context(&old_thread->saved_state.rsp,
                            new_thread->saved_state.rsp);
}
