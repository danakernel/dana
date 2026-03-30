/* kern/exec.h - Executable loading interface
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KERN_EXEC_H
#define KERN_EXEC_H

#include <kern/kern_types.h>
#include <kern/task.h>
#include <stddef.h>

kern_return_t exec_load(task_t task, const void *image_data, size_t image_size,
                        uint64_t *entry_point);
kern_return_t exec_unload(task_t task);

#endif /* KERN_EXEC_H */
