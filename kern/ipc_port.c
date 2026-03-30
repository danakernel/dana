/* kern/ipc_port.c - Mach IPC port implementation
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/ipc_port.h>
#include <kern/ipc_kmsg.h>
#include <vm/zalloc.h>
#include <libkern/string.h>

static zone_t ipc_port_zone;

static zone_t get_ipc_port_zone(void)
{
    if (ipc_port_zone == ZONE_NULL)
        ipc_port_zone = zinit("ipc_port", sizeof(struct ipc_port), 0);
    return ipc_port_zone;
}

kern_return_t ipc_port_alloc(ipc_port_t *port_out)
{
    zone_t z = get_ipc_port_zone();
    if (z == ZONE_NULL)
        return KERN_NO_SPACE;

    struct ipc_port *port = zalloc(z);
    if ((void *)port == (void *)ZONE_NULL)
        return KERN_NO_SPACE;

    kmemset(port, 0, sizeof(struct ipc_port));
    port->ip_type = IPC_PORT_TYPE_RECEIVE;
    port->ip_ref_count = 1;

    *port_out = port;
    return KERN_SUCCESS;
}

void ipc_port_release(ipc_port_t port)
{
    if (port == IPC_PORT_NULL)
        return;

    if (--port->ip_ref_count > 0)
        return;

    while (port->ip_msg_count > 0) {
        struct ipc_kmsg *kmsg = ipc_port_dequeue(port);
        if (kmsg != IPC_KMSG_NULL)
            ipc_kmsg_free(kmsg);
    }

    zfree(get_ipc_port_zone(), port);
}

kern_return_t ipc_port_enqueue(ipc_port_t port, struct ipc_kmsg *kmsg)
{
    if (port == IPC_PORT_NULL || kmsg == IPC_KMSG_NULL)
        return KERN_INVALID_ARGUMENT;

    if (port->ip_msg_count >= IPC_PORT_QUEUE_MAX)
        return KERN_NO_SPACE;

    port->ip_messages[port->ip_msg_last] = kmsg;
    port->ip_msg_last = (port->ip_msg_last + 1) % IPC_PORT_QUEUE_MAX;
    port->ip_msg_count++;

    return KERN_SUCCESS;
}

struct ipc_kmsg *ipc_port_dequeue(ipc_port_t port)
{
    if (port == IPC_PORT_NULL || port->ip_msg_count == 0)
        return IPC_KMSG_NULL;

    struct ipc_kmsg *kmsg = port->ip_messages[port->ip_msg_first];
    port->ip_messages[port->ip_msg_first] = IPC_KMSG_NULL;
    port->ip_msg_first = (port->ip_msg_first + 1) % IPC_PORT_QUEUE_MAX;
    port->ip_msg_count--;

    return kmsg;
}
