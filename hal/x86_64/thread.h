/* hal/x86_64/thread.h - Machine-dependent thread primitives
 * Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Mirrors the role of osfmk/i386/thread.h in XNU: provides the
 * machine-dependent half of the thread context switch interface.
 */

#ifndef HAL_X86_64_THREAD_H
#define HAL_X86_64_THREAD_H

#include <stdint.h>

/* Save callee-saved registers of *old_rsp, switch to new_rsp.
 * Both pointers refer to the saved RSP field in thread_saved_state. */
void machine_switch_context(uint64_t *old_rsp, uint64_t new_rsp);

/* Entry point jumped to when a thread runs for the first time.
 * Expects r15 = entry function, r14 = argument. */
void thread_trampoline(void);

/* Update TSS.rsp0 so the CPU uses the correct kernel stack on ring-0 entry. */
void machine_set_rsp0(uint64_t rsp0);

/* Halt the CPU until the next interrupt. */
void machine_halt(void);

#endif /* HAL_X86_64_THREAD_H */
