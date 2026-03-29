/* hal/x86_64/hal.c - x86_64 HAL implementation
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Implements the hal/hal.h and hal/x86_64/thread.h interfaces for x86_64.
 */

#include <hal/hal.h>
#include <hal/x86_64/io.h>
#include <hal/x86_64/serial.h>
#include <hal/x86_64/vga.h>
#include <hal/x86_64/fb.h>
#include <hal/x86_64/gdt.h>
#include <hal/x86_64/idt.h>
#include <hal/x86_64/cpu.h>
#include <hal/x86_64/thread.h>

void hal_early_console_init(void) {
    serial_init();
}

void hal_console_set_framebuffer(uint64_t addr, uint32_t width, uint32_t height,
                                  uint32_t pitch, uint8_t bpp) {
    fb_init(addr, width, height, pitch, bpp);
}

void hal_console_putc(char c) {
    serial_putc(c);
    if (fb_active())
        fb_putc(c);
}

void hal_init(void) {
    cpu_init();
    cpu_init_self(0);
    gdt_init();
    gdt_load_tss();
    idt_init();
}

void hal_halt(void) {
    cpu_disable_interrupts();
    while (1) {
        cpu_halt();
    }
}

void machine_set_rsp0(uint64_t rsp0) {
    gdt_set_rsp0(rsp0);
}
