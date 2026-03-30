/* kern/ipc_kmsg.c - Kernel internal message handling
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/ipc_kmsg.h>
#include <kern/ipc_port.h>
#include <vm/zalloc.h>
#include <libkern/string.h>

#define KMSG_ZONE_SIZE  512

static zone_t kmsg_zone;

static zone_t get_kmsg_zone(void)
{
    if (kmsg_zone == ZONE_NULL)
        kmsg_zone = zinit("kmsg", sizeof(struct ipc_kmsg) + KMSG_ZONE_SIZE, 0);
    return kmsg_zone;
}

struct ipc_kmsg *ipc_kmsg_alloc(mach_msg_size_t size)
{
    zone_t z = get_kmsg_zone();
    if (z == ZONE_NULL)
        return IPC_KMSG_NULL;

    if (size > KMSG_ZONE_SIZE)
        return IPC_KMSG_NULL;

    struct ipc_kmsg *kmsg = zalloc(z);
    if ((void *)kmsg == (void *)ZONE_NULL)
        return IPC_KMSG_NULL;

    kmsg->ikm_next = IPC_KMSG_NULL;
    kmsg->ikm_port = IPC_PORT_NULL;
    kmsg->ikm_size = size;
    kmsg->ikm_header_size = sizeof(struct mach_msg_header);

    return kmsg;
}

void ipc_kmsg_free(struct ipc_kmsg *kmsg)
{
    if (kmsg == IPC_KMSG_NULL)
        return;

    zfree(get_kmsg_zone(), kmsg);
}

kern_return_t ipc_kmsg_copyin(struct ipc_kmsg *kmsg, const struct mach_msg_header *user_msg,
                               mach_msg_size_t size)
{
    if (kmsg == IPC_KMSG_NULL || (void *)user_msg == (void *)ZONE_NULL)
        return KERN_INVALID_ARGUMENT;

    if (size < sizeof(struct mach_msg_header))
        return KERN_INVALID_ARGUMENT;

    kmsg->ikm_header = *user_msg;
    kmsg->ikm_header_size = sizeof(struct mach_msg_header);

    mach_msg_size_t data_size = size - sizeof(struct mach_msg_header);
    if (data_size > 0) {
        kmemcpy(kmsg->ikm_data, (uint8_t *)user_msg + sizeof(struct mach_msg_header), data_size);
    }

    return KERN_SUCCESS;
}

kern_return_t ipc_kmsg_copyout(struct ipc_kmsg *kmsg, struct mach_msg_header *user_msg,
                                mach_msg_size_t *size)
{
    if (kmsg == IPC_KMSG_NULL || (void *)user_msg == (void *)ZONE_NULL || (void *)size == (void *)ZONE_NULL)
        return KERN_INVALID_ARGUMENT;

    if (kmsg->ikm_size > *size)
        return KERN_NO_SPACE;

    *user_msg = kmsg->ikm_header;

    mach_msg_size_t data_size = kmsg->ikm_size - sizeof(struct mach_msg_header);
    if (data_size > 0)
        kmemcpy((uint8_t *)user_msg + sizeof(struct mach_msg_header), kmsg->ikm_data, data_size);

    *size = kmsg->ikm_size;

    return KERN_SUCCESS;
}
