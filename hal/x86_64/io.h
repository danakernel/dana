/* hal/x86_64/io.h - x86 I/O port operations
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef HAL_X86_64_IO_H
#define HAL_X86_64_IO_H

#include <stdint.h>

static inline void cpu_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" :: "a"(value), "d"(port));
}

static inline void cpu_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile ("outw %0, %1" :: "a"(value), "d"(port));
}

static inline void cpu_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile ("outl %0, %1" :: "a"(value), "d"(port));
}

static inline uint8_t cpu_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "d"(port));
    return value;
}

static inline uint16_t cpu_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "d"(port));
    return value;
}

static inline uint32_t cpu_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "d"(port));
    return value;
}

static inline void cpu_pause(void)
{
    __asm__ volatile ("pause" ::: "memory");
}

static inline void cpu_io_wait(void)
{
    __asm__ volatile ("jmp 1f\n1: jmp 2f\n2:" ::: "memory");
}

static inline void cpu_disable_interrupts(void)
{
    __asm__ volatile ("cli" ::: "memory");
}

static inline void cpu_enable_interrupts(void)
{
    __asm__ volatile ("sti" ::: "memory");
}

#endif /* HAL_X86_64_IO_H */
