/* kern/sched.c - Cooperative scheduler implementation
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/sched.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <hal/x86_64/cpu.h>
#include <hal/thread.h>

static struct sched_runqueue runqueue;

void sched_init(void)
{
    runqueue.head = THREAD_NULL;
    runqueue.tail = THREAD_NULL;
    runqueue.count = 0;
}

kern_return_t sched_enqueue(thread_t thread)
{
    if (thread == THREAD_NULL)
        return KERN_INVALID_ARGUMENT;

    thread->sched.next = THREAD_NULL;
    thread->sched.prev = runqueue.tail;

    if (runqueue.tail != THREAD_NULL)
        runqueue.tail->sched.next = thread;
    else
        runqueue.head = thread;

    runqueue.tail = thread;
    runqueue.count++;

    return KERN_SUCCESS;
}

kern_return_t sched_dequeue(thread_t thread)
{
    if (thread == THREAD_NULL)
        return KERN_INVALID_ARGUMENT;

    if (thread->sched.prev != THREAD_NULL)
        thread->sched.prev->sched.next = thread->sched.next;
    else
        runqueue.head = thread->sched.next;

    if (thread->sched.next != THREAD_NULL)
        thread->sched.next->sched.prev = thread->sched.prev;
    else
        runqueue.tail = thread->sched.prev;

    thread->sched.next = THREAD_NULL;
    thread->sched.prev = THREAD_NULL;
    runqueue.count--;

    return KERN_SUCCESS;
}

thread_t sched_choose(void)
{
    if (runqueue.head != THREAD_NULL)
        return runqueue.head;

    thread_t idle = cpu_idle_thread();
    if (idle != THREAD_NULL)
        return idle;

    return THREAD_NULL;
}

void sched_set_state(thread_t thread, enum thread_state state)
{
    if (thread == THREAD_NULL)
        return;
    thread->sched.state = state;
}

enum thread_state sched_get_state(thread_t thread)
{
    if (thread == THREAD_NULL)
        return THREAD_STATE_TERMINATED;
    return thread->sched.state;
}

thread_t sched_current(void)
{
    return cpu_current_thread();
}

void sched_block(thread_t thread)
{
    if (thread == THREAD_NULL)
        return;

    sched_set_state(thread, THREAD_STATE_BLOCKED);
    sched_dequeue(thread);
}

void sched_unblock(thread_t thread)
{
    if (thread == THREAD_NULL)
        return;

    sched_set_state(thread, THREAD_STATE_RUNNABLE);
    sched_enqueue(thread);
}

void thread_yield(void)
{
    thread_t next = sched_choose();
    thread_t current = cpu_current_thread();

    if (next != THREAD_NULL && next != current) {
        sched_run();
    }
}

void sched_idle(void)
{
    for (;;) {
        thread_t next = sched_choose();
        thread_t current = cpu_current_thread();

        if (next != THREAD_NULL && next != current) {
            sched_run();
        } else {
            cpu_halt();
        }
    }
}

void sched_run(void)
{
    thread_t next = sched_choose();
    thread_t current = cpu_current_thread();

    if (next == THREAD_NULL || next == current)
        return;

    if (current != THREAD_NULL) {
        if (current->sched.state == THREAD_STATE_RUNNING)
            sched_set_state(current, THREAD_STATE_RUNNABLE);
    }

    sched_set_state(next, THREAD_STATE_RUNNING);

    if (next->sched.prev != THREAD_NULL || next->sched.next != THREAD_NULL ||
        runqueue.head == next) {
        sched_dequeue(next);
        sched_enqueue(next);
    }

    cpu_set_current_thread(next);

    thread_switch(current, next);
}

static void idle_thread_entry(void *arg)
{
    (void)arg;
    sched_idle();
}

kern_return_t sched_create_idle(void)
{
    thread_t idle;
    kern_return_t kr;

    kr = thread_create(TASK_NULL, idle_thread_entry, THREAD_NULL, &idle);
    if (kr != KERN_SUCCESS)
        return kr;

    cpu_set_idle_thread(idle);
    cpu_set_current_thread(idle);
    sched_set_state(idle, THREAD_STATE_RUNNING);

    return KERN_SUCCESS;
}
