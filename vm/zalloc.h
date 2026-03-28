/* vm/zalloc.h - Mach-style zone allocator
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * A zone is a pool of fixed-size kernel objects backed by physical pages
 * from the PMM.  Free elements are linked via an embedded freelist pointer
 * stored in the first word of each free slot.
 *
 * Mirrors the interface from XNU's osfmk/kern/zalloc.h.
 */

#ifndef VM_ZALLOC_H
#define VM_ZALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <vm/vm_types.h>

struct zone {
    const char *name;
    vm_size_t   elem_size;
    vm_size_t   max_size;
    vm_size_t   cur_size;
    void       *free_list;
    uint32_t    count_free;
    uint32_t    count_alloc;
};

typedef struct zone *zone_t;

#define ZONE_NULL ((zone_t)0)

zone_t zinit(const char *name, vm_size_t elem_size, vm_size_t max_size);
void  *zalloc(zone_t zone);
void   zfree(zone_t zone, void *elem);

#endif /* VM_ZALLOC_H */
