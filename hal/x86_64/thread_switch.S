; hal/x86_64/thread_switch.S - x86_64 thread context switch
; Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
; SPDX-License-Identifier: GPL-3.0-or-later
;
; machine_switch_context(uint64_t *old_rsp, uint64_t new_rsp)
;   rdi = pointer to old thread's saved_state.rsp
;   rsi = new thread's saved_state.rsp
;
; Saves all callee-saved registers (per SysV AMD64 ABI: rbp, rbx, r12-r15)
; onto the current stack, stores RSP into *old_rsp, loads RSP from new_rsp,
; restores callee-saved registers, and returns into the new thread.
;
; thread_trampoline
;   Entry point for a thread's first run (jumped to via ret).
;   r15 = entry function pointer, r14 = argument (void *).
;
; machine_halt
;   Halt the CPU until the next interrupt (cli; hlt).

bits 64

global machine_switch_context
global thread_trampoline
global machine_halt

machine_switch_context:
    push    rbp
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15

    mov     [rdi], rsp
    mov     rsp, rsi

    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    pop     rbp

    ret

thread_trampoline:
    sti
    mov     rdi, r14
    call    r15
.spin:
    cli
    hlt
    jmp     .spin

machine_halt:
    cli
    hlt
    ret
