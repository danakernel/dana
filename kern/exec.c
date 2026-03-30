/* kern/exec.c - Executable loading interface
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/exec.h>
#include <kern/macho.h>
#include <kern/task.h>
#include <vm/vm_map.h>
#include <hal/pmap.h>
#include <libkern/printf.h>

#define USER_MAP_MIN  0x0000000000001000ULL
#define USER_MAP_MAX  0x00007fffffffffffULL

kern_return_t exec_load(task_t task, const void *image_data, size_t image_size,
                        uint64_t *entry_point)
{
    if (task == TASK_NULL || (void *)image_data == (void *)0 || (void *)entry_point == (void *)0)
        return KERN_INVALID_ARGUMENT;

    if (task->map == VM_MAP_NULL) {
        pmap_t *pmap = pmap_create();
        if (pmap == (void *)0)
            return KERN_NO_SPACE;
        task->map = vm_map_create(pmap, USER_MAP_MIN, USER_MAP_MAX);
        if (task->map == VM_MAP_NULL) {
            pmap_destroy(pmap);
            return KERN_NO_SPACE;
        }
    }

    struct macho_image macho;
    kern_return_t kr = macho_load(task->map, image_data, image_size, &macho);
    if (kr != KERN_SUCCESS) {
        kprintf("EXEC: macho_load failed: %d\n", kr);
        return kr;
    }

    *entry_point = macho.entry_point;

    kprintf("EXEC: loaded task=%p entry=0x%lx\n",
            (void *)task, (unsigned long)*entry_point);

    return KERN_SUCCESS;
}

kern_return_t exec_unload(task_t task)
{
    if (task == TASK_NULL)
        return KERN_INVALID_ARGUMENT;

    if (task->map != VM_MAP_NULL) {
        vm_map_destroy(task->map);
        task->map = VM_MAP_NULL;
    }

    return KERN_SUCCESS;
}
