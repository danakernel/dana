/* kern/ipc_space.h - Mach IPC port space
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Each task has an ipc_space that maps port names to ipc_port pointers.
 * This implements the port right namespace for a task.
 */

#ifndef KERN_IPC_SPACE_H
#define KERN_IPC_SPACE_H

#include <kern/kern_types.h>
#include <kern/ipc_port.h>

#define IPC_SPACE_MAX_PORTS 1024

struct ipc_entry {
    ipc_port_t    ie_port;
    mach_port_t   ie_name;
    uint32_t      ie_rights;
};

struct ipc_space {
    uint32_t          is_port_count;
    struct ipc_entry  is_entries[IPC_SPACE_MAX_PORTS];
};

kern_return_t ipc_space_create(struct ipc_space **space_out);
void          ipc_space_destroy(struct ipc_space *space);
kern_return_t ipc_space_insert(struct ipc_space *space, ipc_port_t port,
                                mach_port_t *name_out);
kern_return_t ipc_space_lookup(struct ipc_space *space, mach_port_t name,
                                ipc_port_t *port_out);
kern_return_t ipc_space_remove(struct ipc_space *space, mach_port_t name);

#endif /* KERN_IPC_SPACE_H */
