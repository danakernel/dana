/* hal/x86_64/vga.c - VGA text mode driver
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <hal/x86_64/vga.h>
#include <hal/x86_64/io.h>

#define VGA_BUF ((volatile uint16_t *)0xb8000)

static int vga_row;
static int vga_col;
static uint8_t vga_attr;

static inline uint16_t vga_entry(char c, uint8_t attr) {
    return (uint16_t)c | ((uint16_t)attr << 8);
}

static void vga_update_cursor(void) {
    uint16_t pos = (uint16_t)(vga_row * VGA_COLS + vga_col);
    cpu_outb(0x3d4, 0x0f);
    cpu_outb(0x3d5, (uint8_t)(pos & 0xff));
    cpu_outb(0x3d4, 0x0e);
    cpu_outb(0x3d5, (uint8_t)(pos >> 8));
}

static void vga_scroll(void) {
    for (int row = 1; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            VGA_BUF[(row - 1) * VGA_COLS + col] = VGA_BUF[row * VGA_COLS + col];
        }
    }
    for (int col = 0; col < VGA_COLS; col++) {
        VGA_BUF[(VGA_ROWS - 1) * VGA_COLS + col] = vga_entry(' ', vga_attr);
    }
    vga_row = VGA_ROWS - 1;
}

void vga_clear(void) {
    for (int i = 0; i < VGA_ROWS * VGA_COLS; i++) {
        VGA_BUF[i] = vga_entry(' ', vga_attr);
    }
    vga_row = 0;
    vga_col = 0;
    vga_update_cursor();
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_attr = (uint8_t)((bg << 4) | (fg & 0x0f));
}

void vga_init(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            VGA_BUF[vga_row * VGA_COLS + vga_col] = vga_entry(' ', vga_attr);
        }
    } else {
        VGA_BUF[vga_row * VGA_COLS + vga_col] = vga_entry(c, vga_attr);
        vga_col++;
        if (vga_col >= VGA_COLS) {
            vga_col = 0;
            vga_row++;
        }
    }

    if (vga_row >= VGA_ROWS) {
        vga_scroll();
    }

    vga_update_cursor();
}
