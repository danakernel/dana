/* hal/x86_64/idt.c - Interrupt Descriptor Table setup
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Installs stubs for the first 32 CPU exception vectors (0-31).
 * IRQ handlers and spurious interrupts will be added in Phase 3.
 */

#include <hal/x86_64/idt.h>
#include <hal/x86_64/gdt.h>
#include <hal/x86_64/io.h>
#include <hal/x86_64/cpu.h>
#include <libkern/printf.h>
#include <vm/vm_fault.h>
#include <vm/vm_map.h>
#include <stdint.h>

#define IDT_SIZE 256

struct idt_entry {
    uint16_t offset_lo;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_hi;
    uint32_t zero;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_SIZE];

static void idt_set(int vec, uint64_t handler, uint8_t type_attr) {
    idt[vec].offset_lo  = handler & 0xffff;
    idt[vec].selector   = GDT_KERNEL_CODE;
    idt[vec].ist        = 0;
    idt[vec].type_attr  = type_attr;
    idt[vec].offset_mid = (handler >> 16) & 0xffff;
    idt[vec].offset_hi  = (handler >> 32) & 0xffffffff;
    idt[vec].zero       = 0;
}

static const char *exception_names[32] = {
    "Division Error",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD FPU Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection",
    "VMM Communication",
    "Security Exception",
    "Reserved",
};

struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

static void exception_handler(uint64_t vector, uint64_t error_code,
                               struct interrupt_frame *frame)
{
    const char *name = (vector < 32) ? exception_names[vector] : "Unknown";
    kprintf("DANA: EXCEPTION #%lu (%s) error=0x%lx rip=0x%lx\n",
            (unsigned long)vector, name,
            (unsigned long)error_code, (unsigned long)frame->rip);
    cpu_disable_interrupts();
    while (1) {
        cpu_halt();
    }
}

#define DEFINE_ISR(n) \
    static void __attribute__((interrupt)) isr_##n(struct interrupt_frame *f) { \
        exception_handler(n, 0, f); \
    }

#define DEFINE_ISR_ERR(n) \
    static void __attribute__((interrupt)) isr_##n(struct interrupt_frame *f, uint64_t e) { \
        exception_handler(n, e, f); \
    }

DEFINE_ISR(0)  DEFINE_ISR(1)  DEFINE_ISR(2)  DEFINE_ISR(3)
DEFINE_ISR(4)  DEFINE_ISR(5)  DEFINE_ISR(6)  DEFINE_ISR(7)
DEFINE_ISR_ERR(8)
DEFINE_ISR(9)
DEFINE_ISR_ERR(10) DEFINE_ISR_ERR(11) DEFINE_ISR_ERR(12)
DEFINE_ISR_ERR(13)

static void __attribute__((interrupt))
isr_14(struct interrupt_frame *frame, uint64_t error_code)
{
    uint64_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

    int fault_type = VM_FAULT_READ;
    if (error_code & 2)  fault_type = VM_FAULT_WRITE;
    if (error_code & 16) fault_type = VM_FAULT_EXECUTE;

    vm_fault_return_t ret = vm_fault(kernel_map, (vm_address_t)cr2, fault_type);
    if (ret == VM_FAULT_SUCCESS)
        return;

    kprintf("DANA: PAGE FAULT cr2=0x%lx error=0x%lx rip=0x%lx [%s]\n",
            (unsigned long)cr2,
            (unsigned long)error_code,
            (unsigned long)frame->rip,
            ret == VM_FAULT_BAD_ADDRESS ? "bad address" :
            ret == VM_FAULT_PROTECTION  ? "protection"  : "oom");
    cpu_disable_interrupts();
    while (1) { cpu_halt(); }
}
DEFINE_ISR(15) DEFINE_ISR(16)
DEFINE_ISR_ERR(17)
DEFINE_ISR(18) DEFINE_ISR(19) DEFINE_ISR(20)
DEFINE_ISR_ERR(21)
DEFINE_ISR(22) DEFINE_ISR(23) DEFINE_ISR(24) DEFINE_ISR(25)
DEFINE_ISR(26) DEFINE_ISR(27) DEFINE_ISR(28) DEFINE_ISR(29)
DEFINE_ISR_ERR(30)
DEFINE_ISR(31)

#define GATE_INT 0x8e

void idt_init(void) {
    idt_set(0,  (uint64_t)isr_0,  GATE_INT);
    idt_set(1,  (uint64_t)isr_1,  GATE_INT);
    idt_set(2,  (uint64_t)isr_2,  GATE_INT);
    idt_set(3,  (uint64_t)isr_3,  GATE_INT);
    idt_set(4,  (uint64_t)isr_4,  GATE_INT);
    idt_set(5,  (uint64_t)isr_5,  GATE_INT);
    idt_set(6,  (uint64_t)isr_6,  GATE_INT);
    idt_set(7,  (uint64_t)isr_7,  GATE_INT);
    idt_set(8,  (uint64_t)isr_8,  GATE_INT);
    idt_set(9,  (uint64_t)isr_9,  GATE_INT);
    idt_set(10, (uint64_t)isr_10, GATE_INT);
    idt_set(11, (uint64_t)isr_11, GATE_INT);
    idt_set(12, (uint64_t)isr_12, GATE_INT);
    idt_set(13, (uint64_t)isr_13, GATE_INT);
    idt_set(14, (uint64_t)isr_14, GATE_INT);
    idt_set(15, (uint64_t)isr_15, GATE_INT);
    idt_set(16, (uint64_t)isr_16, GATE_INT);
    idt_set(17, (uint64_t)isr_17, GATE_INT);
    idt_set(18, (uint64_t)isr_18, GATE_INT);
    idt_set(19, (uint64_t)isr_19, GATE_INT);
    idt_set(20, (uint64_t)isr_20, GATE_INT);
    idt_set(21, (uint64_t)isr_21, GATE_INT);
    idt_set(22, (uint64_t)isr_22, GATE_INT);
    idt_set(23, (uint64_t)isr_23, GATE_INT);
    idt_set(24, (uint64_t)isr_24, GATE_INT);
    idt_set(25, (uint64_t)isr_25, GATE_INT);
    idt_set(26, (uint64_t)isr_26, GATE_INT);
    idt_set(27, (uint64_t)isr_27, GATE_INT);
    idt_set(28, (uint64_t)isr_28, GATE_INT);
    idt_set(29, (uint64_t)isr_29, GATE_INT);
    idt_set(30, (uint64_t)isr_30, GATE_INT);
    idt_set(31, (uint64_t)isr_31, GATE_INT);

    struct idtr idtr = {
        .limit = sizeof(idt) - 1,
        .base  = (uint64_t)&idt,
    };
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}
