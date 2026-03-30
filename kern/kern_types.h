/* kern/kern_types.h - Fundamental Mach kernel types
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Mirrors the names used in XNU's osfmk/mach/mach_types.h and
 * osfmk/kern/kern_types.h so that future source compatibility work
 * has a stable namespace to build on.
 */

#ifndef KERN_KERN_TYPES_H
#define KERN_KERN_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int kern_return_t;

#define KERN_SUCCESS            0
#define KERN_INVALID_ARGUMENT   4
#define KERN_NO_SPACE           3
#define KERN_NOT_SUPPORTED      46
#define KERN_FAILURE            5

typedef unsigned long   natural_t;
typedef long            integer_t;

typedef natural_t       mach_port_name_t;
typedef mach_port_name_t mach_port_t;
typedef natural_t       mach_msg_timeout_t;

typedef kern_return_t   mach_msg_return_t;

typedef struct task    *task_t;
typedef struct thread  *thread_t;
typedef struct ipc_port *ipc_port_t;
typedef struct vm_map  *vm_map_t;
typedef struct ipc_space *ipc_space_t;
struct zone;
typedef struct zone    *zone_t;

#define TASK_NULL       ((task_t)    0)
#define THREAD_NULL     ((thread_t)  0)
#define IPC_PORT_NULL   ((ipc_port_t)0)
#define VM_MAP_NULL     ((vm_map_t)  0)
#define MACH_PORT_NULL  ((mach_port_t)0)
#define IPC_SPACE_NULL  ((ipc_space_t)0)

#endif /* KERN_KERN_TYPES_H */
