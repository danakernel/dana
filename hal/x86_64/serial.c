/* hal/x86_64/serial.c - COM1 UART driver (115200 8N1)
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <hal/x86_64/serial.h>
#include <hal/x86_64/io.h>

#define COM1 0x3f8

void serial_init(void) {
    cpu_outb(COM1 + 1, 0x00);   /* disable interrupts */
    cpu_outb(COM1 + 3, 0x80);   /* enable DLAB */
    cpu_outb(COM1 + 0, 0x01);   /* divisor lo: 115200 baud */
    cpu_outb(COM1 + 1, 0x00);   /* divisor hi */
    cpu_outb(COM1 + 3, 0x03);   /* 8 bits, no parity, 1 stop bit */
    cpu_outb(COM1 + 2, 0xc7);   /* enable FIFO, clear, 14-byte threshold */
    cpu_outb(COM1 + 4, 0x0b);   /* RTS/DSR */
}

static int serial_tx_empty(void) {
    return cpu_inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    while (!serial_tx_empty()) {
        cpu_pause();
    }
    if (c == '\n') {
        cpu_outb(COM1, '\r');
        while (!serial_tx_empty()) {
            cpu_pause();
        }
    }
    cpu_outb(COM1, (uint8_t)c);
}

void serial_puts(const char *s) {
    while (*s) {
        serial_putc(*s++);
    }
}
