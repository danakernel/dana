/* hal/x86_64/pmap.h - x86_64 4-level page table management
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef HAL_X86_64_PMAP_H
#define HAL_X86_64_PMAP_H

#include <stdint.h>
#include <stddef.h>

/* Page table entry flags */
#define PTE_P       (1ULL << 0)   /* Present */
#define PTE_W       (1ULL << 1)   /* Writable */
#define PTE_U       (1ULL << 2)   /* User-accessible */
#define PTE_PWT     (1ULL << 3)   /* Write-through */
#define PTE_PCD     (1ULL << 4)   /* Cache-disable */
#define PTE_A       (1ULL << 5)   /* Accessed */
#define PTE_D       (1ULL << 6)   /* Dirty */
#define PTE_PS      (1ULL << 7)   /* Page size (2 MB in PD, 1 GB in PDPT) */
#define PTE_G       (1ULL << 8)   /* Global */
#define PTE_NX      (1ULL << 63)  /* No-execute */

#define PTE_ADDR_MASK   0x000ffffffffff000ULL

/* Protection shorthand */
#define PMAP_PROT_RO    (PTE_P | PTE_G)
#define PMAP_PROT_RW    (PTE_P | PTE_W | PTE_G)
#define PMAP_PROT_RX    (PTE_P | PTE_G)
#define PMAP_PROT_USER  (PTE_P | PTE_W | PTE_U)

/* Virtual address decomposition */
#define PML4_IDX(va)  (((uint64_t)(va) >> 39) & 0x1ff)
#define PDPT_IDX(va)  (((uint64_t)(va) >> 30) & 0x1ff)
#define PD_IDX(va)    (((uint64_t)(va) >> 21) & 0x1ff)
#define PT_IDX(va)    (((uint64_t)(va) >> 12) & 0x1ff)

/* Physical ↔ virtual for identity-mapped low memory (< 1 GB) */
#define PHYS_TO_VIRT(pa)  ((void *)(uintptr_t)(pa))
#define VIRT_TO_PHYS(va)  ((uint64_t)(uintptr_t)(va))

typedef uint64_t pte_t;

/*
 * pmap_t — machine-dependent address space descriptor.
 * Each task has one; pmap_kernel is shared by all kernel mappings.
 */
typedef struct pmap {
    uint64_t pml4_phys;   /* physical address of the PML4 table */
    uint32_t ref_count;   /* reference count */
} pmap_t;

/* Kernel pmap — initialized by pmap_init(), shared across all tasks. */
extern pmap_t pmap_kernel;

/* Lifecycle */
void    pmap_init(void);
pmap_t *pmap_create(void);
void    pmap_destroy(pmap_t *pmap);
void    pmap_reference(pmap_t *pmap);

/* Activation */
void    pmap_switch(pmap_t *pmap);

/* Mapping */
int     pmap_map(pmap_t *pmap, uint64_t vaddr, uint64_t paddr, uint64_t flags);
int     pmap_unmap(pmap_t *pmap, uint64_t vaddr);
int     pmap_protect(pmap_t *pmap, uint64_t vaddr, uint64_t flags);
uint64_t pmap_query(pmap_t *pmap, uint64_t vaddr);   /* returns paddr or 0 */

/* Internal — used by pmm bootstrap and vm layer */
uint64_t pmap_alloc_table(void);

#endif /* HAL_X86_64_PMAP_H */
