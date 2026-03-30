/* vm/vm_map.h - Mach vm_map: per-task address space descriptor
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef VM_VM_MAP_H
#define VM_VM_MAP_H

#include <kern/kern_types.h>
#include <vm/vm_types.h>
#include <hal/pmap.h>

/* vm_map_entry flags */
#define VME_WIRED   (1u << 0)   /* pages are wired in; no demand fault */

struct vm_map_entry {
    vm_address_t         start;
    vm_address_t         end;       /* exclusive */
    vm_prot_t            prot;
    vm_prot_t            max_prot;
    uint32_t             flags;
    struct vm_map_entry *prev;
    struct vm_map_entry *next;
};

/* vm_map_t is typedef'd in kern/kern_types.h as struct vm_map * */
struct vm_map {
    vm_address_t         min_offset;
    vm_address_t         max_offset;
    uint32_t             ref_count;
    uint32_t             nentries;
    struct vm_map_entry  header;    /* sentinel: header.next = first, header.prev = last */
    pmap_t              *pmap;
};

/* The kernel address space. Initialised by vm_map_init(). */
extern struct vm_map vm_map_kernel_store;
#define kernel_map (&vm_map_kernel_store)

kern_return_t        vm_map_init(void);
vm_map_t             vm_map_create(pmap_t *pmap, vm_address_t min, vm_address_t max);
void                 vm_map_destroy(vm_map_t map);

kern_return_t        vm_map_enter(vm_map_t map,
                                  vm_address_t start, vm_size_t size,
                                  vm_prot_t prot, vm_prot_t max_prot,
                                  uint32_t flags);

kern_return_t        vm_map_remove(vm_map_t map,
                                   vm_address_t start, vm_size_t size);

kern_return_t        vm_map_page_range(vm_map_t map,
                                       vm_address_t start, vm_size_t size,
                                       vm_prot_t prot);

kern_return_t        vm_deallocate(vm_map_t map,
                                   vm_address_t start, vm_size_t size);

struct vm_map_entry *vm_map_lookup_entry(vm_map_t map, vm_address_t addr);

#endif /* VM_VM_MAP_H */
