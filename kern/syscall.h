/* kern/syscall.h - Mach syscall interface
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Mach trap numbers and syscall interface.
 * Uses XNU-style negative syscall numbering.
 */

#ifndef KERN_SYSCALL_H
#define KERN_SYSCALL_H

#include <kern/kern_types.h>
#include <stdint.h>

struct mach_msg_header;

#define SYSCALL_MACH_MSG        1
#define SYSCALL_THREAD_CREATE   2
#define SYSCALL_THREAD_EXIT     3
#define SYSCALL_TASK_CREATE     4
#define SYSCALL_VM_ALLOCATE     5
#define SYSCALL_VM_DEALLOCATE   6
#define SYSCALL_EXEC            7

void syscall_init(void);
kern_return_t syscall_mach_msg(struct mach_msg_header *msg,
                                uint32_t option,
                                uint32_t send_size,
                                uint32_t rcv_size,
                                mach_port_t rcv_name,
                                mach_msg_timeout_t timeout,
                                mach_port_t notify);

typedef kern_return_t (*syscall_handler_t)(uint64_t arg1, uint64_t arg2,
                                            uint64_t arg3, uint64_t arg4,
                                            uint64_t arg5, uint64_t arg6);

kern_return_t syscall_dispatch(int syscall_num, uint64_t arg1, uint64_t arg2,
                                uint64_t arg3, uint64_t arg4, uint64_t arg5,
                                uint64_t arg6);

#endif /* KERN_SYSCALL_H */
