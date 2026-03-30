/* kern/ipc_space.c - Mach IPC port space implementation
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/ipc_space.h>
#include <vm/zalloc.h>
#include <libkern/string.h>

static zone_t ipc_space_zone;

static zone_t get_ipc_space_zone(void)
{
    if (ipc_space_zone == ZONE_NULL)
        ipc_space_zone = zinit("ipc_space", sizeof(struct ipc_space), 0);
    return ipc_space_zone;
}

kern_return_t ipc_space_create(struct ipc_space **space_out)
{
    zone_t z = get_ipc_space_zone();
    if (z == ZONE_NULL)
        return KERN_NO_SPACE;

    struct ipc_space *space = zalloc(z);
    if ((void *)space == (void *)ZONE_NULL)
        return KERN_NO_SPACE;

    kmemset(space, 0, sizeof(struct ipc_space));
    *space_out = space;
    return KERN_SUCCESS;
}

void ipc_space_destroy(struct ipc_space *space)
{
    if ((void *)space == (void *)ZONE_NULL)
        return;

    for (uint32_t i = 0; i < space->is_port_count; i++) {
        if (space->is_entries[i].ie_port != IPC_PORT_NULL) {
            ipc_port_release(space->is_entries[i].ie_port);
        }
    }

    zfree(get_ipc_space_zone(), space);
}

kern_return_t ipc_space_insert(struct ipc_space *space, ipc_port_t port,
                                mach_port_t *name_out)
{
    if ((void *)space == (void *)ZONE_NULL || port == IPC_PORT_NULL || (void *)name_out == (void *)ZONE_NULL)
        return KERN_INVALID_ARGUMENT;

    if (space->is_port_count >= IPC_SPACE_MAX_PORTS)
        return KERN_NO_SPACE;

    mach_port_t name = space->is_port_count + 1;
    space->is_entries[space->is_port_count].ie_port = port;
    space->is_entries[space->is_port_count].ie_name = name;
    space->is_entries[space->is_port_count].ie_rights = 1;
    space->is_port_count++;

    *name_out = name;
    return KERN_SUCCESS;
}

kern_return_t ipc_space_lookup(struct ipc_space *space, mach_port_t name,
                                ipc_port_t *port_out)
{
    if ((void *)space == (void *)ZONE_NULL || (void *)port_out == (void *)ZONE_NULL)
        return KERN_INVALID_ARGUMENT;

    if (name == MACH_PORT_NULL || name > space->is_port_count)
        return KERN_INVALID_ARGUMENT;

    struct ipc_entry *entry = &space->is_entries[name - 1];
    if (entry->ie_port == IPC_PORT_NULL)
        return KERN_INVALID_ARGUMENT;

    *port_out = entry->ie_port;
    return KERN_SUCCESS;
}

kern_return_t ipc_space_remove(struct ipc_space *space, mach_port_t name)
{
    if ((void *)space == (void *)ZONE_NULL)
        return KERN_INVALID_ARGUMENT;

    if (name == MACH_PORT_NULL || name > space->is_port_count)
        return KERN_INVALID_ARGUMENT;

    struct ipc_entry *entry = &space->is_entries[name - 1];
    if (entry->ie_port != IPC_PORT_NULL) {
        ipc_port_release(entry->ie_port);
        entry->ie_port = IPC_PORT_NULL;
        entry->ie_rights = 0;
    }

    return KERN_SUCCESS;
}
