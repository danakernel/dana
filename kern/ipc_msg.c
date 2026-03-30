/* kern/ipc_msg.c - Mach IPC message handling
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/ipc_msg.h>
#include <kern/ipc_kmsg.h>
#include <kern/ipc_port.h>
#include <kern/ipc_space.h>
#include <kern/task.h>
#include <kern/sched.h>
#include <kern/thread.h>
#include <vm/zalloc.h>

kern_return_t mach_msg_send(struct mach_msg_header *msg, mach_msg_size_t size)
{
    if ((void *)msg == (void *)ZONE_NULL || size < sizeof(struct mach_msg_header))
        return KERN_INVALID_ARGUMENT;

    mach_port_t remote_port = msg->msgh_remote_port;
    if (remote_port == MACH_PORT_NULL)
        return KERN_INVALID_ARGUMENT;

    task_t current_task = sched_current()->task;
    if (current_task == TASK_NULL || current_task->itk_space == IPC_SPACE_NULL)
        return KERN_INVALID_ARGUMENT;

    ipc_port_t port = IPC_PORT_NULL;
    kern_return_t kr = ipc_space_lookup(current_task->itk_space, remote_port, &port);
    if (kr != KERN_SUCCESS)
        return kr;

    struct ipc_kmsg *kmsg = ipc_kmsg_alloc(size);
    if (kmsg == IPC_KMSG_NULL)
        return KERN_NO_SPACE;

    kr = ipc_kmsg_copyin(kmsg, msg, size);
    if (kr != KERN_SUCCESS) {
        ipc_kmsg_free(kmsg);
        return kr;
    }

    kmsg->ikm_port = port;

    kr = ipc_port_enqueue(port, kmsg);
    if (kr != KERN_SUCCESS) {
        ipc_kmsg_free(kmsg);
        return kr;
    }

    return KERN_SUCCESS;
}

kern_return_t mach_msg_receive(struct mach_msg_header *msg, mach_msg_size_t *size)
{
    if ((void *)msg == (void *)ZONE_NULL || (void *)size == (void *)ZONE_NULL)
        return KERN_INVALID_ARGUMENT;

    mach_port_t local_port = msg->msgh_local_port;
    if (local_port == MACH_PORT_NULL)
        return KERN_INVALID_ARGUMENT;

    task_t current_task = sched_current()->task;
    if (current_task == TASK_NULL || current_task->itk_space == IPC_SPACE_NULL)
        return KERN_INVALID_ARGUMENT;

    ipc_port_t port = IPC_PORT_NULL;
    kern_return_t kr = ipc_space_lookup(current_task->itk_space, local_port, &port);
    if (kr != KERN_SUCCESS)
        return kr;

    if (port->ip_msg_count == 0) {
        thread_t thread = sched_current();
        sched_block(thread);
        return KERN_SUCCESS;
    }

    struct ipc_kmsg *kmsg = ipc_port_dequeue(port);
    if (kmsg == IPC_KMSG_NULL)
        return KERN_FAILURE;

    kr = ipc_kmsg_copyout(kmsg, msg, size);
    ipc_kmsg_free(kmsg);

    return kr;
}

kern_return_t mach_msg(struct mach_msg_header *msg, mach_msg_bits_t option,
                       mach_msg_size_t send_size, mach_msg_size_t rcv_size,
                       mach_port_t rcv_name, mach_msg_timeout_t timeout,
                       mach_port_t notify, struct mach_msg_header *rcv_msg)
{
    kern_return_t kr;

    if (option & 1) {
        kr = mach_msg_send(msg, send_size);
        if (kr != KERN_SUCCESS)
            return kr;
    }

    if (option & 2) {
        if ((void *)rcv_msg != (void *)ZONE_NULL)
            rcv_msg->msgh_local_port = rcv_name;
        kr = mach_msg_receive((void *)rcv_msg != (void *)ZONE_NULL ? rcv_msg : msg, &rcv_size);
        if (kr != KERN_SUCCESS)
            return kr;
    }

    return KERN_SUCCESS;
}
