/* kern/task.h - Mach task structure
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * A task is the unit of resource ownership in Mach: it owns a virtual
 * address space, a set of threads, and a port namespace.
 */

#ifndef KERN_TASK_H
#define KERN_TASK_H

#include <kern/kern_types.h>

struct ipc_space;

struct task {
    uint32_t        task_id;
    uint32_t        ref_count;
    vm_map_t        map;
    thread_t        threads;
    ipc_port_t      itk_self;
    uint32_t        thread_count;
    struct ipc_space *itk_space;
};

kern_return_t task_create(task_t parent, bool inherit_memory, task_t *child_out);
kern_return_t task_destroy(task_t task);

extern struct task kernel_task;

#endif /* KERN_TASK_H */
