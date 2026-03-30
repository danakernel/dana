/* kern/ipc_msg.h - Mach IPC message structures
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Mach message header and related structures.
 * Based on XNU's osfmk/mach/message.h
 */

#ifndef KERN_IPC_MSG_H
#define KERN_IPC_MSG_H

#include <kern/kern_types.h>
#include <stdint.h>

typedef natural_t mach_msg_bits_t;
typedef natural_t mach_msg_size_t;
typedef natural_t mach_msg_id_t;
typedef natural_t mach_msg_type_name_t;
typedef natural_t mach_port_right_t;

#define MACH_MSG_TYPE_MAKE_SEND       20
#define MACH_MSG_TYPE_MAKE_SEND_ONCE  21
#define MACH_MSG_TYPE_COPY_SEND       22
#define MACH_MSG_TYPE_MOVE_SEND       23

#define MACH_MSG_TYPE_RECEIVE         24
#define MACH_MSG_TYPE_MOVE_RECEIVE    25

#define MACH_MSG_TYPE_PORT_NONE       0
#define MACH_MSG_TYPE_PORT_NAME       1
#define MACH_MSG_TYPE_PORT_RECEIVE    2
#define MACH_MSG_TYPE_PORT_SEND       3
#define MACH_MSG_TYPE_PORT_SEND_ONCE  4

#define MACH_MSGH_BITS_COMPLEX (1u << 8)
#define MACH_MSGH_BITS_REMOTE(bits) ((bits) & 0xff)
#define MACH_MSGH_BITS_LOCAL(bits)  (((bits) >> 8) & 0xff)
#define MACH_MSGH_BITS(remote, local) ((remote) | ((local) << 8))

struct mach_msg_header {
    mach_msg_bits_t       msgh_bits;
    mach_msg_size_t       msgh_size;
    mach_port_t           msgh_remote_port;
    mach_port_t           msgh_local_port;
    mach_port_t           msgh_voucher_port;
    mach_msg_id_t         msgh_id;
};

struct mach_msg_trailer {
    mach_msg_size_t       msgh_trailer_size;
    mach_msg_size_t       msgh_trailer_type;
    uint64_t              msgh_sender;
};

struct mach_msg_base {
    struct mach_msg_header header;
    union {
        uint8_t             data[0];
        struct mach_msg_trailer trailer;
    } body;
};

#define MACH_MSG_SIZE_MAX   0x10000
#define MACH_MSG_HEADER_MIN sizeof(struct mach_msg_header)

kern_return_t mach_msg_send(struct mach_msg_header *msg, mach_msg_size_t size);
kern_return_t mach_msg_receive(struct mach_msg_header *msg, mach_msg_size_t *size);
kern_return_t mach_msg(struct mach_msg_header *msg, mach_msg_bits_t option,
                       mach_msg_size_t send_size, mach_msg_size_t rcv_size,
                       mach_port_t rcv_name, mach_msg_timeout_t timeout,
                       mach_port_t notify, struct mach_msg_header *rcv_msg);

#endif /* KERN_IPC_MSG_H */
