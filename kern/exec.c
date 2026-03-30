/* kern/exec.c - Executable loading interface
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <kern/exec.h>
#include <kern/macho.h>
#include <kern/task.h>
#include <vm/vm_map.h>
#include <libkern/printf.h>

kern_return_t exec_load(task_t task, const void *image_data, size_t image_size,
                        uint64_t *entry_point)
{
    if (task == TASK_NULL || (void *)image_data == (void *)0 || (void *)entry_point == (void *)0)
        return KERN_INVALID_ARGUMENT;

    struct macho_image macho;
    kern_return_t kr = macho_load(image_data, image_size, &macho);
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
