/* vm/pmm.h - Physical memory manager (bitmap allocator)
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef VM_PMM_H
#define VM_PMM_H

#include <stdint.h>
#include <stddef.h>
#include <multiboot2.h>

#define PMM_PAGE_SIZE   4096UL
#define PMM_PAGE_SHIFT  12

#define PMM_NULL        0UL

void     pmm_init(struct multiboot2_tag_mmap *mmap);
uint64_t pmm_alloc_page(void);
void     pmm_free_page(uint64_t paddr);
size_t   pmm_free_pages(void);
size_t   pmm_total_pages(void);

#endif /* VM_PMM_H */
