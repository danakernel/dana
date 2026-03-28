#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include <stdint.h>

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

#define MULTIBOOT2_TAG_TYPE_END               0
#define MULTIBOOT2_TAG_TYPE_MMAP              6
#define MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO     4

#define MULTIBOOT2_MEMORY_AVAILABLE           1
#define MULTIBOOT2_MEMORY_RESERVED            2
#define MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE    3
#define MULTIBOOT2_MEMORY_NVS                 4
#define MULTIBOOT2_MEMORY_BADRAM              5

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot2_info {
    uint32_t total_size;
    uint32_t reserved;
};

struct multiboot2_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
};

struct multiboot2_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot2_mmap_entry entries[0];
};

struct multiboot2_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

#endif /* MULTIBOOT2_H */
