/* kern/ipc_kmsg.h - Kernel internal message structure
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Kernel internal message wrapper that holds a copy of the user message
 * and metadata for IPC processing.
 */

#ifndef KERN_IPC_KMSG_H
#define KERN_IPC_KMSG_H

#include <kern/ipc_msg.h>
#include <stddef.h>

struct ipc_port;

struct ipc_kmsg {
    struct ipc_kmsg      *ikm_next;
    struct ipc_port      *ikm_port;
    mach_msg_size_t       ikm_size;
    mach_msg_size_t       ikm_header_size;
    struct mach_msg_header ikm_header;
    uint8_t               ikm_data[];
};

struct ipc_kmsg *ipc_kmsg_alloc(mach_msg_size_t size);
void ipc_kmsg_free(struct ipc_kmsg *kmsg);
kern_return_t ipc_kmsg_copyin(struct ipc_kmsg *kmsg, const struct mach_msg_header *user_msg,
                               mach_msg_size_t size);
kern_return_t ipc_kmsg_copyout(struct ipc_kmsg *kmsg, struct mach_msg_header *user_msg,
                                mach_msg_size_t *size);

#define IPC_KMSG_NULL ((struct ipc_kmsg *)0)

#endif /* KERN_IPC_KMSG_H */
