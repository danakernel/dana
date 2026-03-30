; hal/x86_64/syscall.S - x86_64 syscall entry/exit
; Copyright (C) 2026 AnmiTaliDev <anmitalidev@nuros.org>
; SPDX-License-Identifier: GPL-3.0-or-later
;
; syscall entry point - called via syscall instruction
; on entry:
;   r10 = rcx (userland saves return address in rcx)
;   rax = syscall number
;   rdi, rsi, rdx, r10, r8, r9 = arguments
; on exit:
;   rax = return value
;   rdx = error flag (0 = success)

bits 64

global syscall_entry
global syscall_handler

extern syscall_dispatch

syscall_entry:
    ; Save user registers
    push    rax             ; syscall number
    push    rcx             ; user RIP
    push    r11             ; user RFLAGS
    push    rbp
    push    rdi
    push    rsi
    push    rdx
    push    r10
    push    r8
    push    r9

    ; Align stack to 16 bytes
    sub     rsp, 8

    ; Call syscall_dispatch(rax, rdi, rsi, rdx, r10, r8, r9)
    ; Arguments already in correct registers
    mov     rdi, rax        ; syscall number
    mov     rsi, [rsp + 56] ; arg1 (original rdi)
    mov     rdx, [rsp + 64] ; arg2 (original rsi)
    mov     rcx, [rsp + 72] ; arg3 (original rdx)
    mov     r8,  [rsp + 80] ; arg4 (original r10)
    mov     r9,  [rsp + 88] ; arg5 (original r8)
    push    qword [rsp + 96]; arg6 (original r9)

    call    syscall_dispatch

    add     rsp, 16         ; clean up pushed arg6 + alignment

    ; Restore registers
    pop     r9
    pop     r8
    pop     r10
    pop     rdx
    pop     rsi
    pop     rdi
    pop     rbp
    pop     r11
    pop     rcx
    pop     rax

    ; Return to userland
    sysret

syscall_handler:
    ret
