# GNUmakefile - DANA kernel build system
# Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
# SPDX-License-Identifier: GPL-3.0-or-later

HAL ?= x86_64

CC      := gcc
AS      := nasm
LD      := ld

CFLAGS  := -std=c11 \
           -ffreestanding \
           -fno-stack-protector \
           -fno-stack-check \
           -fno-pie \
           -fno-pic \
           -mno-80387 \
           -mno-mmx \
           -mno-sse \
           -mno-sse2 \
           -mno-red-zone \
           -mcmodel=kernel \
           -Wall \
           -Wextra \
           -Wno-unused-parameter \
           -I. \
           -Iinclude \
           -Ikern \
           -Ivm \
           -Ilibkern \
           -Ihal \
           -Ihal/$(HAL)

ASFLAGS := -f elf64

LDFLAGS := -nostdlib \
           -static \
           -m elf_x86_64 \
           -z max-page-size=0x1000 \
           -T hal/$(HAL)/kernel.ld

BUILD       := build
ISOROOT     := $(BUILD)/isoroot
LIMINE_DIR  := limine
LIMINE_TOOL := $(LIMINE_DIR)/limine
OVMF        := /usr/share/edk2/x64/OVMF.4m.fd

SRCS_C  := main.c \
           kern/task.c \
           kern/thread.c \
           kern/sched.c \
           kern/clock.c \
           kern/ipc_port.c \
           libkern/string.c \
           libkern/printf.c \
           vm/pmm.c \
           vm/vm_map.c \
           vm/vm_fault.c \
           vm/zalloc.c \
           hal/$(HAL)/serial.c \
           hal/$(HAL)/vga.c \
           hal/$(HAL)/fb.c \
           hal/$(HAL)/gdt.c \
           hal/$(HAL)/idt.c \
           hal/$(HAL)/hal.c \
           hal/$(HAL)/pmap.c \
           hal/$(HAL)/cpu.c \
           hal/$(HAL)/pit.c

SRCS_S  := hal/$(HAL)/boot_entry.S \
           hal/$(HAL)/thread_switch.S

OBJS    := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C)) \
           $(patsubst %.S, $(BUILD)/%.o, $(SRCS_S))

KERNEL  := $(BUILD)/dana.elf
ISO     := $(BUILD)/dana.iso

LIMINE_BASE := https://raw.githubusercontent.com/limine-bootloader/limine/v10.x-binary

.PHONY: all build iso run run-uefi get-deps clean

all: build

build: $(KERNEL)

$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

iso: build
	rm -rf $(ISOROOT)
	mkdir -p $(ISOROOT)/boot/limine $(ISOROOT)/EFI/BOOT
	cp $(KERNEL)                              $(ISOROOT)/boot/dana.elf
	cp hal/$(HAL)/limine.conf                 $(ISOROOT)/boot/limine/limine.conf
	cp $(LIMINE_DIR)/limine-bios.sys          $(ISOROOT)/boot/limine/
	cp $(LIMINE_DIR)/limine-bios-cd.bin       $(ISOROOT)/boot/limine/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin       $(ISOROOT)/boot/limine/
	cp $(LIMINE_DIR)/BOOTX64.EFI              $(ISOROOT)/EFI/BOOT/
	xorriso -as mkisofs -R -r -J \
	    -b boot/limine/limine-bios-cd.bin \
	    -no-emul-boot -boot-load-size 4 -boot-info-table \
	    --efi-boot boot/limine/limine-uefi-cd.bin \
	    -efi-boot-part --efi-boot-image \
	    $(ISOROOT) -o $(ISO)
	$(LIMINE_TOOL) bios-install $(ISO)

run: iso
	qemu-system-x86_64 \
	    -M q35 \
	    -m 512M \
	    -cdrom $(ISO) \
	    -boot d \
	    -serial stdio \
	    -d cpu_reset,guest_errors \
	    -D qemu.log

run-uefi: iso
	qemu-system-x86_64 \
	    -M q35 \
	    -m 512M \
	    -bios $(OVMF) \
	    -cdrom $(ISO) \
	    -boot d \
	    -serial stdio \
	    -d cpu_reset,guest_errors \
	    -D qemu.log

clean:
	rm -rf $(BUILD) qemu.log
