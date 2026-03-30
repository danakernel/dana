/* kern/macho.c - Mach-O binary loader
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/macho.h>
#include <kern/task.h>
#include <vm/vm_map.h>
#include <vm/pmm.h>
#include <vm/zalloc.h>
#include <libkern/string.h>
#include <libkern/printf.h>

static kern_return_t macho_validate(struct mach_header_64 *header)
{
    if ((void *)header == (void *)0)
        return KERN_INVALID_ARGUMENT;

    if (header->magic != MH_MAGIC_64)
        return KERN_INVALID_ARGUMENT;

    if (header->filetype != MH_EXECUTE && header->filetype != MH_OBJECT)
        return KERN_INVALID_ARGUMENT;

    if (header->cputype != CPU_TYPE_X86_64)
        return KERN_NOT_SUPPORTED;

    return KERN_SUCCESS;
}

static kern_return_t macho_parse_segments(struct mach_header_64 *header,
                                          const void *image_data,
                                          struct macho_image *image)
{
    struct load_command *lc = (struct load_command *)((uint8_t *)header + sizeof(struct mach_header_64));
    kern_return_t kr;

    for (uint32_t i = 0; i < header->ncmds; i++) {
        if (lc->cmd == LC_SEGMENT_64) {
            struct segment_command_64 *seg = (struct segment_command_64 *)lc;

            if (seg->filesize > 0) {
                vm_address_t addr = seg->vmaddr;
                vm_size_t size = seg->vmsize;
                vm_prot_t prot = 0;

                if (seg->initprot & VM_PROT_READ)
                    prot |= VM_PROT_READ;
                if (seg->initprot & VM_PROT_WRITE)
                    prot |= VM_PROT_WRITE;
                if (seg->initprot & VM_PROT_EXECUTE)
                    prot |= VM_PROT_EXECUTE;

                kr = vm_map_page_range(kernel_map, addr, size, prot);
                if (kr != KERN_SUCCESS)
                    return kr;

                const void *src = (const uint8_t *)image_data + seg->fileoff;
                void *dst = (void *)(uintptr_t)addr;
                kmemcpy(dst, src, seg->filesize);

                if (seg->vmsize > seg->filesize) {
                    kmemset((uint8_t *)dst + seg->filesize, 0, seg->vmsize - seg->filesize);
                }
            }
        }

        lc = (struct load_command *)((uint8_t *)lc + lc->cmdsize);
    }

    return KERN_SUCCESS;
}

static kern_return_t macho_find_entry(struct mach_header_64 *header,
                                      struct macho_image *image)
{
    struct load_command *lc = (struct load_command *)((uint8_t *)header + sizeof(struct mach_header_64));

    for (uint32_t i = 0; i < header->ncmds; i++) {
        if (lc->cmd == LC_MAIN) {
            struct entry_point_command *ep = (struct entry_point_command *)lc;
            image->entry_point = ep->entryoff;
            return KERN_SUCCESS;
        }

        lc = (struct load_command *)((uint8_t *)lc + lc->cmdsize);
    }

    image->entry_point = 0;
    return KERN_SUCCESS;
}

kern_return_t macho_load(const void *image_data, size_t image_size,
                         struct macho_image *image_out)
{
    if ((void *)image_data == (void *)0 || (void *)image_out == (void *)0)
        return KERN_INVALID_ARGUMENT;

    if (image_size < sizeof(struct mach_header_64))
        return KERN_INVALID_ARGUMENT;

    struct mach_header_64 *header = (struct mach_header_64 *)image_data;

    kern_return_t kr = macho_validate(header);
    if (kr != KERN_SUCCESS)
        return kr;

    image_out->header = header;
    image_out->size = image_size;
    image_out->base_address = 0;

    kr = macho_parse_segments(header, image_data, image_out);
    if (kr != KERN_SUCCESS)
        return kr;

    kr = macho_find_entry(header, image_out);
    if (kr != KERN_SUCCESS)
        return kr;

    kprintf("MACHO: loaded entry=0x%lx size=%lu\n",
            (unsigned long)image_out->entry_point,
            (unsigned long)image_size);

    return KERN_SUCCESS;
}

kern_return_t macho_unload(struct macho_image *image)
{
    if ((void *)image == (void *)0 || (void *)image->header == (void *)0)
        return KERN_INVALID_ARGUMENT;

    struct load_command *lc = (struct load_command *)((uint8_t *)image->header + sizeof(struct mach_header_64));

    for (uint32_t i = 0; i < image->header->ncmds; i++) {
        if (lc->cmd == LC_SEGMENT_64) {
            struct segment_command_64 *seg = (struct segment_command_64 *)lc;

            if (seg->vmsize > 0) {
                vm_deallocate(kernel_map, seg->vmaddr, seg->vmsize);
            }
        }

        lc = (struct load_command *)((uint8_t *)lc + lc->cmdsize);
    }

    image->header = (void *)0;
    image->entry_point = 0;
    image->size = 0;

    return KERN_SUCCESS;
}
