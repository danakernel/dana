/* vm/pmm.c - Physical memory manager (bitmap allocator)
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * One bit per 4 KB page; 0 = free, 1 = used.
 * The bitmap itself is placed in the first available region
 * above the kernel (physical > 1 MB).
 */

#include <stddef.h>
#include <stdint.h>
#include <multiboot2.h>
#include <vm/pmm.h>
#include <libkern/string.h>
#include <libkern/printf.h>

#define BITS_PER_WORD   64UL
#define WORD_IDX(page)  ((page) / BITS_PER_WORD)
#define BIT_IDX(page)   ((page) % BITS_PER_WORD)

/* Physical address of the kernel end — defined in kernel.ld */
extern uint8_t _kernel_end;
#define KERN_END_PHYS  ((uint64_t)(uintptr_t)&_kernel_end - 0xffffffff80000000ULL)

/* Maximum physical memory we track: 4 GB (1M pages) */
#define PMM_MAX_PAGES   (1UL << 20)
#define PMM_BITMAP_WORDS (PMM_MAX_PAGES / BITS_PER_WORD)

static uint64_t *bitmap;         /* pointer to the bitmap (physical = virtual for low mem) */
static size_t    total_pages;
static size_t    free_count;

static void bitmap_set(size_t page)
{
    bitmap[WORD_IDX(page)] |= (1ULL << BIT_IDX(page));
}

static void bitmap_clear(size_t page)
{
    bitmap[WORD_IDX(page)] &= ~(1ULL << BIT_IDX(page));
}

static int bitmap_test(size_t page)
{
    return (bitmap[WORD_IDX(page)] >> BIT_IDX(page)) & 1;
}

void pmm_init(struct multiboot2_tag_mmap *mmap)
{
    /* Place the bitmap right after the kernel image, page-aligned. */
    uint64_t bitmap_phys = (KERN_END_PHYS + PMM_PAGE_SIZE - 1) & ~(PMM_PAGE_SIZE - 1);
    size_t   bitmap_bytes = PMM_BITMAP_WORDS * sizeof(uint64_t);

    bitmap = (uint64_t *)(uintptr_t)bitmap_phys;

    /* Mark everything used (1) initially. */
    kmemset(bitmap, 0xff, bitmap_bytes);
    total_pages = 0;
    free_count  = 0;

    uint8_t *entry_ptr = (uint8_t *)mmap->entries;
    uint8_t *end_ptr   = (uint8_t *)mmap + mmap->size;

    while (entry_ptr < end_ptr) {
        struct multiboot2_mmap_entry *e = (struct multiboot2_mmap_entry *)entry_ptr;

        if (e->type == MULTIBOOT2_MEMORY_AVAILABLE) {
            uint64_t start = (e->addr + PMM_PAGE_SIZE - 1) & ~(PMM_PAGE_SIZE - 1);
            uint64_t end   = (e->addr + e->len) & ~(PMM_PAGE_SIZE - 1);

            for (uint64_t addr = start; addr < end; addr += PMM_PAGE_SIZE) {
                size_t page = (size_t)(addr >> PMM_PAGE_SHIFT);
                if (page >= PMM_MAX_PAGES)
                    break;
                total_pages++;
                bitmap_clear(page);
                free_count++;
            }
        }

        entry_ptr += mmap->entry_size;
    }

    /* Reserve page 0 (NULL trap). */
    bitmap_set(0);
    if (free_count > 0) free_count--;

    /* Reserve the bitmap region itself. */
    uint64_t bm_start = bitmap_phys;
    uint64_t bm_end   = bitmap_phys + bitmap_bytes;
    for (uint64_t addr = bm_start; addr < bm_end; addr += PMM_PAGE_SIZE) {
        size_t page = (size_t)(addr >> PMM_PAGE_SHIFT);
        if (page < PMM_MAX_PAGES && !bitmap_test(page)) {
            bitmap_set(page);
            if (free_count > 0) free_count--;
        }
    }

    /* Reserve everything below 1 MB (BIOS, VGA, etc.). */
    for (uint64_t addr = 0; addr < 0x100000; addr += PMM_PAGE_SIZE) {
        size_t page = (size_t)(addr >> PMM_PAGE_SHIFT);
        if (page < PMM_MAX_PAGES && !bitmap_test(page)) {
            bitmap_set(page);
            if (free_count > 0) free_count--;
        }
    }

    /* Reserve kernel image pages. */
    uint64_t kern_start = 0x200000;
    uint64_t kern_end   = (KERN_END_PHYS + PMM_PAGE_SIZE - 1) & ~(PMM_PAGE_SIZE - 1);
    for (uint64_t addr = kern_start; addr < kern_end; addr += PMM_PAGE_SIZE) {
        size_t page = (size_t)(addr >> PMM_PAGE_SHIFT);
        if (page < PMM_MAX_PAGES && !bitmap_test(page)) {
            bitmap_set(page);
            if (free_count > 0) free_count--;
        }
    }

    kprintf("PMM: %u MB free (%u total pages)\n",
            (unsigned)(free_count >> 8),   /* pages / 256 ≈ MiB */
            (unsigned)total_pages);
}

uint64_t pmm_alloc_page(void)
{
    for (size_t w = 0; w < PMM_BITMAP_WORDS; w++) {
        if (bitmap[w] == ~0ULL)
            continue;
        int bit = __builtin_ctzll(~bitmap[w]);
        size_t page = w * BITS_PER_WORD + (size_t)bit;
        if (page >= PMM_MAX_PAGES)
            break;
        bitmap_set(page);
        if (free_count > 0) free_count--;
        return (uint64_t)page << PMM_PAGE_SHIFT;
    }
    return PMM_NULL;
}

void pmm_free_page(uint64_t paddr)
{
    size_t page = (size_t)(paddr >> PMM_PAGE_SHIFT);
    if (page == 0 || page >= PMM_MAX_PAGES)
        return;
    bitmap_clear(page);
    free_count++;
}

size_t pmm_free_pages(void)  { return free_count; }
size_t pmm_total_pages(void) { return total_pages; }
