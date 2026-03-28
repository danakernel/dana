/* hal/x86_64/hal.c - x86_64 HAL implementation
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Implements the hal/hal.h interface for the x86_64 platform.
 */

#include <hal/hal.h>
#include <hal/x86_64/serial.h>
#include <hal/x86_64/vga.h>
#include <hal/x86_64/gdt.h>
#include <hal/x86_64/idt.h>
#include <hal/x86_64/cpu.h>

void hal_early_console_init(void) {
    serial_init();
    vga_init();
}

void hal_console_putc(char c) {
    serial_putc(c);
    vga_putc(c);
}

void hal_init(void) {
    gdt_init();
    idt_init();
}

void hal_halt(void) {
    cpu_disable_interrupts();
    while (1) {
        cpu_halt();
    }
}
