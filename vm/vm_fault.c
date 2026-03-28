/* vm/vm_fault.c - Mach page fault resolution
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * vm_fault() is the single entry point from the architecture-specific
 * page-fault trap handler.  It resolves anonymous demand-paged faults:
 *
 *   1. Find the vm_map_entry that covers the faulting address.
 *   2. Verify the access is allowed by the entry's protection.
 *   3. Allocate a zeroed physical page from the PMM.
 *   4. Install the mapping via pmap_map().
 *
 * Wired pages never fault (they were mapped at vm_map_enter time).
 * Copy-on-write and file-backed objects belong to Phase 4+.
 */

#include <vm/vm_fault.h>
#include <vm/vm_map.h>
#include <vm/pmm.h>
#include <hal/pmap.h>
#include <libkern/string.h>

static uint64_t prot_to_pte(vm_prot_t prot)
{
    uint64_t f = 0;
    if (prot & VM_PROT_WRITE)
        f |= PTE_W;
    if (!(prot & VM_PROT_EXECUTE))
        f |= PTE_NX;
    return f;
}

vm_fault_return_t vm_fault(vm_map_t map, vm_address_t addr, int fault_type)
{
    vm_address_t page_addr = addr & ~(vm_address_t)(PMM_PAGE_SIZE - 1);

    /* Spurious fault — page is already mapped (e.g. racing CPU). */
    if (pmap_query(map->pmap, page_addr) != 0)
        return VM_FAULT_SUCCESS;

    struct vm_map_entry *entry = vm_map_lookup_entry(map, addr);
    if (entry == NULL)
        return VM_FAULT_BAD_ADDRESS;

    if (fault_type == VM_FAULT_WRITE && !(entry->prot & VM_PROT_WRITE))
        return VM_FAULT_PROTECTION;
    if (fault_type == VM_FAULT_EXECUTE && !(entry->prot & VM_PROT_EXECUTE))
        return VM_FAULT_PROTECTION;

    uint64_t paddr = pmm_alloc_page();
    if (paddr == PMM_NULL)
        return VM_FAULT_OOM;

    kmemset(PHYS_TO_VIRT(paddr), 0, PMM_PAGE_SIZE);

    if (pmap_map(map->pmap, page_addr, paddr, prot_to_pte(entry->prot)) != 0) {
        pmm_free_page(paddr);
        return VM_FAULT_OOM;
    }

    return VM_FAULT_SUCCESS;
}
