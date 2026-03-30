/* kern/macho.h - Mach-O binary format structures
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Mach-O 64-bit format structures for loading Darwin/macOS binaries.
 * Based on XNU's mach-o/loader.h
 */

#ifndef KERN_MACHO_H
#define KERN_MACHO_H

#include <kern/kern_types.h>
#include <vm/vm_map.h>
#include <vm/vm_types.h>
#include <stdint.h>

#define MH_MAGIC_64     0xfeedfacf
#define MH_CIGAM_64     0xcffaedfe

#define MH_OBJECT       0x1
#define MH_EXECUTE      0x2
#define MH_DYLIB        0x6
#define MH_DYLINKER     0x7
#define MH_BUNDLE       0x8
#define MH_DYLIB_STUB   0x9
#define MH_DSYM         0xa

#define MH_NOUNDEFS     0x1
#define MH_PIE          0x200000
#define MH_TWOLEVEL     0x800000

#define LC_SEGMENT_64       0x19
#define LC_LOAD_DYLIB       0xc
#define LC_LOAD_DYLINKER    0xe
#define LC_UNIXTHREAD       0x05
#define LC_MAIN             0x80000028

struct mach_header_64 {
    uint32_t magic;
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
};

struct load_command {
    uint32_t cmd;
    uint32_t cmdsize;
};

struct segment_command_64 {
    uint32_t cmd;
    uint32_t cmdsize;
    char     segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    uint32_t maxprot;
    uint32_t initprot;
    uint32_t nsects;
    uint32_t flags;
};

struct section_64 {
    char     sectname[16];
    char     segname[16];
    uint64_t addr;
    uint64_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
};

struct thread_command {
    uint32_t cmd;
    uint32_t cmdsize;
};

struct entry_point_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint64_t entryoff;
    uint64_t stacksize;
};

#define CPU_TYPE_X86_64     0x01000007
#define CPU_SUBTYPE_AMD64_ALL 0x00000003

struct macho_image {
    struct mach_header_64 *header;
    uint64_t              entry_point;
    uint64_t              base_address;
    uint64_t              size;
};

kern_return_t macho_load(vm_map_t map, const void *image_data, size_t image_size,
                         struct macho_image *image_out);
kern_return_t macho_unload(struct macho_image *image);

#endif /* KERN_MACHO_H */
