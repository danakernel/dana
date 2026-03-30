/* main.c - DANA kernel C entry point (Limine protocol)
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <hal/hal.h>
#include <hal/x86_64/io.h>
#include <hal/x86_64/pmap.h>
#include <libkern/printf.h>
#include <kern/task.h>
#include <kern/sched.h>
#include <kern/clock.h>
#include <kern/syscall.h>
#include <vm/pmm.h>
#include <vm/vm_map.h>

uint64_t hhdm_offset;

__attribute__((section(".limine_requests_start")))
static volatile uint64_t requests_start[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((section(".limine_requests_end")))
static volatile uint64_t requests_end[] = LIMINE_REQUESTS_END_MARKER;

__attribute__((section(".limine_requests")))
static volatile uint64_t base_revision[] = LIMINE_BASE_REVISION(3);

__attribute__((section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = 0
};

__attribute__((section(".limine_requests")))
static volatile struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = 0
};

__attribute__((section(".limine_requests")))
static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = 0
};

__attribute__((section(".limine_requests")))
static volatile struct limine_executable_address_request exe_req = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
    .response = 0
};

__attribute__((section(".limine_requests")))
static volatile struct limine_stack_size_request stack_req = {
    .id = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision = 0,
    .response = 0,
    .stack_size = 256 * 1024
};

void kmain(void) {
    hal_early_console_init();

    kprintf("DANA: booting...\n");
    kprintf("DANA: DANA is Almost Not Apple\n");

    if (!LIMINE_BASE_REVISION_SUPPORTED(base_revision)) {
        kprintf("DANA: ERROR: Limine base revision not supported\n");
        hal_halt();
    }

    if (!hhdm_req.response) {
        kprintf("DANA: ERROR: no HHDM response\n");
        hal_halt();
    }
    hhdm_offset = hhdm_req.response->offset;
    kprintf("DANA: HHDM offset = 0x%llx\n", (unsigned long long)hhdm_offset);

    if (fb_req.response && fb_req.response->framebuffer_count > 0) {
        struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
        if (fb->memory_model == LIMINE_FRAMEBUFFER_RGB) {
            hal_console_set_framebuffer(
                (uint64_t)(uintptr_t)fb->address,
                (uint32_t)fb->width,
                (uint32_t)fb->height,
                (uint32_t)fb->pitch,
                (uint8_t)fb->bpp);
            kprintf("DANA: framebuffer %ux%u %ubpp\n",
                    (unsigned)fb->width, (unsigned)fb->height, (unsigned)fb->bpp);
        }
    }

    if (!memmap_req.response) {
        kprintf("DANA: ERROR: no memory map\n");
        hal_halt();
    }
    kprintf("DANA: memory map has %u entries\n",
            (unsigned)memmap_req.response->entry_count);

    uint64_t kern_phys_start = 0x200000;
    uint64_t kern_phys_end   = 0x200000;
    if (exe_req.response) {
        kern_phys_start = exe_req.response->physical_base;
        extern uint8_t _kernel_end;
        kern_phys_end = kern_phys_start +
            ((uint64_t)(uintptr_t)&_kernel_end - exe_req.response->virtual_base);
    }

    hal_init();
    kprintf("DANA: GDT and IDT loaded\n");

    pmm_init((struct limine_memmap_response *)memmap_req.response,
             kern_phys_start, kern_phys_end);
    pmap_init();
    vm_map_init();

    kprintf("DANA: kernel task id = %u\n", kernel_task.task_id);

    sched_init();
    clock_init();
    sched_create_idle();

    kprintf("DANA: boot complete\n");

    cpu_enable_interrupts();
    sched_idle();
}
