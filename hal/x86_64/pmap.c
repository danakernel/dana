/* hal/x86_64/pmap.c - x86_64 4-level page table management
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Design
 * ------
 * pmap_t wraps a PML4 table (one per address space).
 * pmap_kernel is initialised from the CR3 left by boot_entry.S —
 * the boot-time mappings (identity 0-1 GB + higher-half) are kept
 * intact so that physical addresses < 1 GB remain directly
 * addressable as virtual == physical throughout the kernel.
 *
 * All intermediate page-table pages (PDPT / PD / PT) are
 * allocated from the PMM via pmap_alloc_table() and zeroed
 * before use.  Freeing intermediate tables is deferred to
 * pmap_destroy(); individual pmap_unmap() calls only clear PTEs.
 *
 * CR3 layout: bits[63:12] = PML4 physical address, bits[11:0] = 0
 * (PCID not used yet).
 */

#include <stddef.h>
#include <hal/x86_64/pmap.h>
#include <vm/pmm.h>
#include <libkern/string.h>
#include <libkern/printf.h>

pmap_t pmap_kernel;

static inline uint64_t read_cr3(void)
{
    uint64_t v;
    __asm__ volatile("mov %%cr3, %0" : "=r"(v));
    return v;
}

static inline void write_cr3(uint64_t v)
{
    __asm__ volatile("mov %0, %%cr3" :: "r"(v) : "memory");
}

static inline void invlpg(uint64_t va)
{
    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
}

/* Flush the entire TLB by reloading CR3. */
static inline void tlb_flush_all(void)
{
    write_cr3(read_cr3());
}

uint64_t pmap_alloc_table(void)
{
    uint64_t pa = pmm_alloc_page();
    if (pa == PMM_NULL)
        return PMM_NULL;
    kmemset(PHYS_TO_VIRT(pa), 0, PMM_PAGE_SIZE);
    return pa;
}

/*
 * Walk PML4 → PDPT → PD → PT for vaddr.
 * If create == 1, allocate missing intermediate tables.
 * Returns a pointer to the PT entry, or NULL on failure.
 */
static pte_t *pmap_walk(pmap_t *pmap, uint64_t vaddr, int create)
{
    pte_t *pml4 = (pte_t *)PHYS_TO_VIRT(pmap->pml4_phys);

    pte_t *e4 = &pml4[PML4_IDX(vaddr)];
    if (!(*e4 & PTE_P)) {
        if (!create) return NULL;
        uint64_t pa = pmap_alloc_table();
        if (pa == PMM_NULL) return NULL;
        *e4 = pa | PTE_P | PTE_W | PTE_U;
    }
    pte_t *pdpt = (pte_t *)PHYS_TO_VIRT(*e4 & PTE_ADDR_MASK);

    pte_t *e3 = &pdpt[PDPT_IDX(vaddr)];
    if (!(*e3 & PTE_P)) {
        if (!create) return NULL;
        uint64_t pa = pmap_alloc_table();
        if (pa == PMM_NULL) return NULL;
        *e3 = pa | PTE_P | PTE_W | PTE_U;
    }
    pte_t *pd = (pte_t *)PHYS_TO_VIRT(*e3 & PTE_ADDR_MASK);

    pte_t *e2 = &pd[PD_IDX(vaddr)];
    if (!(*e2 & PTE_P)) {
        if (!create) return NULL;
        uint64_t pa = pmap_alloc_table();
        if (pa == PMM_NULL) return NULL;
        *e2 = pa | PTE_P | PTE_W | PTE_U;
    }
    pte_t *pt = (pte_t *)PHYS_TO_VIRT(*e2 & PTE_ADDR_MASK);

    return &pt[PT_IDX(vaddr)];
}

/*
 * Recursively free all page-table pages under a PML4 entry.
 * Does NOT free mapped physical pages — that is the vm_map layer's job.
 */
static void pmap_free_tables(uint64_t pml4_phys)
{
    pte_t *pml4 = (pte_t *)PHYS_TO_VIRT(pml4_phys);

    for (int i4 = 0; i4 < 512; i4++) {
        if (!(pml4[i4] & PTE_P)) continue;
        pte_t *pdpt = (pte_t *)PHYS_TO_VIRT(pml4[i4] & PTE_ADDR_MASK);

        for (int i3 = 0; i3 < 512; i3++) {
            if (!(pdpt[i3] & PTE_P)) continue;
            if (pdpt[i3] & PTE_PS) continue;   /* 1 GB page — no PT below */
            pte_t *pd = (pte_t *)PHYS_TO_VIRT(pdpt[i3] & PTE_ADDR_MASK);

            for (int i2 = 0; i2 < 512; i2++) {
                if (!(pd[i2] & PTE_P)) continue;
                if (pd[i2] & PTE_PS) continue;  /* 2 MB page */
                pmm_free_page(pd[i2] & PTE_ADDR_MASK);
            }
            pmm_free_page(pdpt[i3] & PTE_ADDR_MASK);
        }
        pmm_free_page(pml4[i4] & PTE_ADDR_MASK);
    }
    pmm_free_page(pml4_phys);
}

void pmap_init(void)
{
    uint64_t cr3 = read_cr3() & PTE_ADDR_MASK;
    pmap_kernel.pml4_phys = cr3;
    pmap_kernel.ref_count = 1;
    kprintf("PMAP: kernel PML4 at phys 0x%x\n", (unsigned)cr3);
}

/*
 * Allocate a new pmap for a user task.
 * The upper half (kernel entries) is copied from pmap_kernel so that
 * kernel memory is visible from any address space without per-task
 * mappings — the standard higher-half kernel trick.
 */
pmap_t *pmap_create(void)
{
    uint64_t pa = pmap_alloc_table();
    if (pa == PMM_NULL)
        return NULL;

    /* Copy kernel PML4 entries (upper 256 slots = indices 256–511). */
    pte_t *new_pml4  = (pte_t *)PHYS_TO_VIRT(pa);
    pte_t *kern_pml4 = (pte_t *)PHYS_TO_VIRT(pmap_kernel.pml4_phys);
    for (int i = 256; i < 512; i++)
        new_pml4[i] = kern_pml4[i];

    pmap_t *pmap = (pmap_t *)pmm_alloc_page();
    if (pmap == NULL) {
        pmm_free_page(pa);
        return NULL;
    }
    pmap->pml4_phys = pa;
    pmap->ref_count = 1;
    return pmap;
}

void pmap_reference(pmap_t *pmap)
{
    pmap->ref_count++;
}

void pmap_destroy(pmap_t *pmap)
{
    if (pmap == &pmap_kernel)
        return;
    if (--pmap->ref_count > 0)
        return;

    pmap_free_tables(pmap->pml4_phys);
    pmm_free_page(VIRT_TO_PHYS(pmap));
}

/* Load this pmap's PML4 into CR3, flushing the TLB. */
void pmap_switch(pmap_t *pmap)
{
    write_cr3(pmap->pml4_phys);
}

/*
 * Map vaddr → paddr with given flags in pmap.
 * Returns 0 on success, -1 on OOM.
 */
int pmap_map(pmap_t *pmap, uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
    pte_t *pte = pmap_walk(pmap, vaddr, 1);
    if (pte == NULL)
        return -1;
    *pte = (paddr & PTE_ADDR_MASK) | (flags & ~PTE_ADDR_MASK) | PTE_P;
    invlpg(vaddr);
    return 0;
}

/*
 * Remove the mapping for vaddr.
 * Returns 0 if it was present, -1 otherwise.
 */
int pmap_unmap(pmap_t *pmap, uint64_t vaddr)
{
    pte_t *pte = pmap_walk(pmap, vaddr, 0);
    if (pte == NULL || !(*pte & PTE_P))
        return -1;
    *pte = 0;
    invlpg(vaddr);
    return 0;
}

/*
 * Change protection flags on an existing mapping.
 * The physical address is preserved; only the flag bits change.
 * Returns 0 on success, -1 if the page is not mapped.
 */
int pmap_protect(pmap_t *pmap, uint64_t vaddr, uint64_t flags)
{
    pte_t *pte = pmap_walk(pmap, vaddr, 0);
    if (pte == NULL || !(*pte & PTE_P))
        return -1;
    *pte = (*pte & PTE_ADDR_MASK) | (flags & ~PTE_ADDR_MASK) | PTE_P;
    invlpg(vaddr);
    return 0;
}

/*
 * Translate vaddr → physical address.
 * Returns the physical address, or 0 if not mapped.
 */
uint64_t pmap_query(pmap_t *pmap, uint64_t vaddr)
{
    pte_t *pte = pmap_walk(pmap, vaddr, 0);
    if (pte == NULL || !(*pte & PTE_P))
        return 0;
    return (*pte & PTE_ADDR_MASK) | (vaddr & (PMM_PAGE_SIZE - 1));
}
