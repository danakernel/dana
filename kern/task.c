/* kern/task.c - Mach task management stubs
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/task.h>
#include <kern/ipc_space.h>

struct task kernel_task = {
    .task_id      = 0,
    .ref_count    = 1,
    .map          = VM_MAP_NULL,
    .threads      = THREAD_NULL,
    .itk_self     = IPC_PORT_NULL,
    .thread_count = 0,
    .itk_space    = IPC_SPACE_NULL,
};

kern_return_t task_create(task_t parent, bool inherit_memory, task_t *child_out) {
    (void)parent;
    (void)inherit_memory;
    (void)child_out;
    return KERN_NOT_SUPPORTED;
}

kern_return_t task_destroy(task_t task) {
    (void)task;
    return KERN_NOT_SUPPORTED;
}
