/* vm/zalloc.c - Mach-style zone allocator
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Each zone is a pool of same-sized elements.  Backing pages are allocated
 * from the PMM on demand and divided into elements linked via an embedded
 * freelist pointer in the first word of each free slot.
 *
 * Zone descriptors are drawn from a small static array; the elements
 * themselves live in PMM-allocated pages accessed through HHDM.
 */

#include <vm/zalloc.h>
#include <vm/pmm.h>
#include <hal/pmap.h>
#include <libkern/string.h>
#include <libkern/printf.h>

#define ZONE_MAX 32

static struct zone zone_store[ZONE_MAX];
static int zone_count;

zone_t zinit(const char *name, vm_size_t elem_size, vm_size_t max_size)
{
    if (zone_count >= ZONE_MAX) {
        kprintf("ZONE: zinit: zone table full\n");
        return ZONE_NULL;
    }

    if (elem_size < sizeof(void *))
        elem_size = sizeof(void *);
    elem_size = (elem_size + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);

    struct zone *z = &zone_store[zone_count++];
    z->name        = name;
    z->elem_size   = elem_size;
    z->max_size    = max_size;
    z->cur_size    = 0;
    z->free_list   = NULL;
    z->count_free  = 0;
    z->count_alloc = 0;
    return z;
}

static int zone_grow(zone_t z)
{
    if (z->max_size && z->cur_size >= z->max_size)
        return -1;

    uint64_t paddr = pmm_alloc_page();
    if (paddr == PMM_NULL)
        return -1;

    uint8_t *page  = (uint8_t *)PHYS_TO_VIRT(paddr);
    vm_size_t n    = PMM_PAGE_SIZE / z->elem_size;

    for (vm_size_t i = 0; i < n; i++) {
        void *elem    = page + i * z->elem_size;
        *(void **)elem = z->free_list;
        z->free_list  = elem;
        z->count_free++;
    }
    z->cur_size += PMM_PAGE_SIZE;
    return 0;
}

void *zalloc(zone_t z)
{
    if (z->free_list == NULL) {
        if (zone_grow(z) != 0)
            return NULL;
    }
    void *elem     = z->free_list;
    z->free_list   = *(void **)elem;
    z->count_free--;
    z->count_alloc++;
    kmemset(elem, 0, z->elem_size);
    return elem;
}

void zfree(zone_t z, void *elem)
{
    *(void **)elem = z->free_list;
    z->free_list   = elem;
    z->count_free++;
    z->count_alloc--;
}
