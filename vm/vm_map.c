/* vm/vm_map.c - Mach vm_map implementation
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Address space = doubly-linked list of vm_map_entry, sorted by start
 * address, with a sentinel header node.  Entries and maps are allocated
 * from fixed-size zones backed by the PMM.
 *
 * Kernel address space: [0xffff800000000000, 0xffffffffffffffff].
 * User address space:   [0x0000000000001000, 0x00007fffffffffff].
 */

#include <stddef.h>
#include <vm/vm_map.h>
#include <vm/zalloc.h>
#include <vm/pmm.h>
#include <hal/pmap.h>
#include <libkern/string.h>
#include <libkern/printf.h>

#define KERN_MAP_MIN  0xffff800000000000ULL
#define KERN_MAP_MAX  0xffffffffffffffffULL

static zone_t vm_map_zone;
static zone_t vm_map_entry_zone;

struct vm_map vm_map_kernel_store;

static void map_list_init(vm_map_t map)
{
    map->header.next = &map->header;
    map->header.prev = &map->header;
}

static void map_list_insert_after(struct vm_map_entry *pos,
                                   struct vm_map_entry *ne)
{
    ne->prev       = pos;
    ne->next       = pos->next;
    pos->next->prev = ne;
    pos->next       = ne;
}

static void map_list_remove(struct vm_map_entry *e)
{
    e->prev->next = e->next;
    e->next->prev = e->prev;
}

static uint64_t prot_to_pte(vm_prot_t prot)
{
    uint64_t f = 0;
    if (prot & VM_PROT_WRITE)
        f |= PTE_W;
    if (!(prot & VM_PROT_EXECUTE))
        f |= PTE_NX;
    return f;
}

kern_return_t vm_map_init(void)
{
    vm_map_entry_zone = zinit("vm_map_entry",
                              sizeof(struct vm_map_entry), 0);
    vm_map_zone       = zinit("vm_map",
                              sizeof(struct vm_map), 0);

    vm_map_t km = &vm_map_kernel_store;
    kmemset(km, 0, sizeof(*km));
    km->min_offset = KERN_MAP_MIN;
    km->max_offset = KERN_MAP_MAX;
    km->ref_count  = 1;
    km->pmap       = &pmap_kernel;
    map_list_init(km);
    kprintf("VM: kernel map [0x%llx, 0x%llx]\n",
            (unsigned long long)km->min_offset,
            (unsigned long long)km->max_offset);
    return KERN_SUCCESS;
}

vm_map_t vm_map_create(pmap_t *pmap, vm_address_t min, vm_address_t max)
{
    vm_map_t map = zalloc(vm_map_zone);
    if (map == VM_MAP_NULL)
        return VM_MAP_NULL;
    map->min_offset = min;
    map->max_offset = max;
    map->ref_count  = 1;
    map->pmap       = pmap;
    map_list_init(map);
    return map;
}

void vm_map_destroy(vm_map_t map)
{
    if (map == kernel_map)
        return;
    if (--map->ref_count > 0)
        return;

    struct vm_map_entry *e = map->header.next;
    while (e != &map->header) {
        struct vm_map_entry *next = e->next;
        for (vm_address_t a = e->start; a < e->end; a += PMM_PAGE_SIZE) {
            uint64_t pa = pmap_query(map->pmap, a);
            if (pa) {
                pmap_unmap(map->pmap, a);
                pmm_free_page(pa);
            }
        }
        zfree(vm_map_entry_zone, e);
        e = next;
    }
    pmap_destroy(map->pmap);
    zfree(vm_map_zone, map);
}

struct vm_map_entry *vm_map_lookup_entry(vm_map_t map, vm_address_t addr)
{
    struct vm_map_entry *e = map->header.next;
    while (e != &map->header) {
        if (addr >= e->start && addr < e->end)
            return e;
        e = e->next;
    }
    return NULL;
}

kern_return_t vm_map_enter(vm_map_t map,
                            vm_address_t start, vm_size_t size,
                            vm_prot_t prot, vm_prot_t max_prot,
                            uint32_t flags)
{
    if (size == 0 || start < map->min_offset)
        return KERN_INVALID_ARGUMENT;

    vm_address_t end = start + size;
    if (end > map->max_offset || end < start)
        return KERN_INVALID_ARGUMENT;

    struct vm_map_entry *e = map->header.next;
    while (e != &map->header) {
        if (start < e->end && end > e->start)
            return KERN_NO_SPACE;
        e = e->next;
    }

    struct vm_map_entry *ne = zalloc(vm_map_entry_zone);
    if (ne == NULL)
        return KERN_NO_SPACE;

    ne->start    = start;
    ne->end      = end;
    ne->prot     = prot;
    ne->max_prot = max_prot;
    ne->flags    = flags;

    struct vm_map_entry *pos = &map->header;
    while (pos->next != &map->header && pos->next->start < start)
        pos = pos->next;
    map_list_insert_after(pos, ne);
    map->nentries++;

    if (flags & VME_WIRED) {
        uint64_t pte_flags = prot_to_pte(prot);
        for (vm_address_t a = start; a < end; a += PMM_PAGE_SIZE) {
            uint64_t pa = pmm_alloc_page();
            if (pa == PMM_NULL) {
                vm_map_remove(map, start, size);
                return KERN_NO_SPACE;
            }
            kmemset(PHYS_TO_VIRT(pa), 0, PMM_PAGE_SIZE);
            if (pmap_map(map->pmap, a, pa, pte_flags) != 0) {
                pmm_free_page(pa);
                vm_map_remove(map, start, size);
                return KERN_NO_SPACE;
            }
        }
    }

    return KERN_SUCCESS;
}

kern_return_t vm_map_remove(vm_map_t map,
                             vm_address_t start, vm_size_t size)
{
    vm_address_t end = start + size;
    struct vm_map_entry *e = map->header.next;

    while (e != &map->header) {
        struct vm_map_entry *next = e->next;

        if (e->start >= end || e->end <= start) {
            e = next;
            continue;
        }

        vm_address_t unmap_start = (e->start > start) ? e->start : start;
        vm_address_t unmap_end   = (e->end   < end)   ? e->end   : end;

        for (vm_address_t a = unmap_start; a < unmap_end; a += PMM_PAGE_SIZE) {
            uint64_t pa = pmap_query(map->pmap, a);
            if (pa) {
                pmap_unmap(map->pmap, a);
                pmm_free_page(pa);
            }
        }

        map_list_remove(e);
        zfree(vm_map_entry_zone, e);
        map->nentries--;

        e = next;
    }

    return KERN_SUCCESS;
}
