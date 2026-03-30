/* kern/syscall.c - Mach syscall dispatch
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/syscall.h>
#include <kern/ipc_msg.h>
#include <kern/sched.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <vm/zalloc.h>
#include <libkern/printf.h>

#define MAX_SYSCALL 64

static syscall_handler_t syscall_table[MAX_SYSCALL];

static kern_return_t sys_mach_msg_wrapper(uint64_t arg1, uint64_t arg2,
                                           uint64_t arg3, uint64_t arg4,
                                           uint64_t arg5, uint64_t arg6)
{
    (void)arg5;
    (void)arg6;
    
    return syscall_mach_msg((struct mach_msg_header *)arg1,
                            (uint32_t)arg2,
                            (uint32_t)arg3,
                            (uint32_t)arg4,
                            0, 0, 0);
}

static kern_return_t sys_thread_create_wrapper(uint64_t arg1, uint64_t arg2,
                                                uint64_t arg3, uint64_t arg4,
                                                uint64_t arg5, uint64_t arg6)
{
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;
    (void)arg6;
    
    task_t task = (task_t)arg1;
    thread_t thread;
    kern_return_t kr = thread_create(task, (void (*)(void *))arg3, (void *)arg4, &thread);
    return kr;
}

static kern_return_t sys_thread_exit_wrapper(uint64_t arg1, uint64_t arg2,
                                              uint64_t arg3, uint64_t arg4,
                                              uint64_t arg5, uint64_t arg6)
{
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;
    (void)arg6;
    
    thread_t current = sched_current();
    if (current != THREAD_NULL) {
        thread_destroy(current);
    }
    return KERN_SUCCESS;
}

void syscall_init(void)
{
    for (int i = 0; i < MAX_SYSCALL; i++) {
        syscall_table[i] = ((void *)0);
    }
    
    syscall_table[SYSCALL_MACH_MSG] = sys_mach_msg_wrapper;
    syscall_table[SYSCALL_THREAD_CREATE] = sys_thread_create_wrapper;
    syscall_table[SYSCALL_THREAD_EXIT] = sys_thread_exit_wrapper;
    
    kprintf("DANA: syscall table initialized\n");
}

kern_return_t syscall_dispatch(int syscall_num, uint64_t arg1, uint64_t arg2,
                                uint64_t arg3, uint64_t arg4, uint64_t arg5,
                                uint64_t arg6)
{
    if (syscall_num < 0 || syscall_num >= MAX_SYSCALL) {
        return KERN_NOT_SUPPORTED;
    }
    
    syscall_handler_t handler = syscall_table[syscall_num];
    if (handler == ((void *)0)) {
        return KERN_NOT_SUPPORTED;
    }
    
    return handler(arg1, arg2, arg3, arg4, arg5, arg6);
}

kern_return_t syscall_mach_msg(struct mach_msg_header *msg,
                                uint32_t option,
                                uint32_t send_size,
                                uint32_t rcv_size,
                                mach_port_t rcv_name,
                                mach_msg_timeout_t timeout,
                                mach_port_t notify)
{
    (void)timeout;
    (void)notify;
    
    return mach_msg(msg, option, send_size, rcv_size, rcv_name, 0, 0, msg);
}
