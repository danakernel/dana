/* kern/sched.h - Cooperative scheduler for Mach microkernel
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Cooperative scheduler: threads voluntarily yield control via thread_yield().
 * The scheduler uses a simple round-robin policy among runnable threads.
 */

#ifndef KERN_SCHED_H
#define KERN_SCHED_H

#include <kern/kern_types.h>
#include <stdbool.h>

struct thread;

enum thread_state {
    THREAD_STATE_RUNNABLE,
    THREAD_STATE_RUNNING,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_TERMINATED
};

struct sched_runqueue {
    struct thread *head;
    struct thread *tail;
    uint32_t       count;
};

struct thread_sched {
    enum thread_state   state;
    struct thread      *next;
    struct thread      *prev;
    uint64_t            run_time;
};

void sched_init(void);
kern_return_t sched_enqueue(struct thread *thread);
kern_return_t sched_dequeue(struct thread *thread);
struct thread *sched_choose(void);
void thread_yield(void);
void sched_block(struct thread *thread);
void sched_unblock(struct thread *thread);
struct thread *sched_current(void);
void sched_set_state(struct thread *thread, enum thread_state state);
enum thread_state sched_get_state(struct thread *thread);
void sched_idle(void);
void sched_run(void);
kern_return_t sched_create_idle(void);

#endif /* KERN_SCHED_H */
