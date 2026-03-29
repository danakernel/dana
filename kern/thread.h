/* kern/thread.h - Mach thread structure
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * A thread is the unit of CPU execution. thread_saved_state holds only
 * the kernel RSP; all callee-saved registers live on the kernel stack
 * and are pushed/popped by machine_switch_context.
 */

#ifndef KERN_THREAD_H
#define KERN_THREAD_H

#include <kern/kern_types.h>

struct thread_sched;

struct thread_saved_state {
    uint64_t rsp;
};

struct thread {
    uint32_t                  thread_id;
    uint32_t                  ref_count;
    task_t                    task;
    struct thread            *task_next;
    struct thread_saved_state saved_state;
    ipc_port_t                ith_self;
    uint64_t                  kernel_stack_phys;
    uint64_t                  kernel_stack_top;
    struct thread_sched       sched;
};

kern_return_t thread_create(task_t task, void (*entry)(void *), void *arg,
                             thread_t *thread_out);
kern_return_t thread_destroy(thread_t thread);
void          thread_switch(thread_t old_thread, thread_t new_thread);

#endif /* KERN_THREAD_H */
