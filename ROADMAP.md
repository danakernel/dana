# DANA Roadmap

DANA (Dana is Almost Not Apple) is a Mach microkernel targeting Darwin/XNU ABI
compatibility. 

## Phase 1 — Bootable Skeleton

- Limine boot protocol handoff (x86\_64 HAL)
- HAL interface (`hal/hal.h`) isolating kernel from platform
- GDT and IDT initialization
- COM1 serial UART debug output
- Mach core type stubs: `task`, `thread`, `ipc_port`, `mach_msg_header_t`
- Higher-half kernel linked at `0xffffffff80000000`
- QEMU-runnable ISO image

Completion criteria: kernel prints boot messages over serial and halts cleanly.

## Phase 2 — Memory Management

- Physical memory allocator (bitmap over Limine memory map)
- x86\_64 pmap: 4-level page table management
- Kernel virtual memory manager: `vm_map`, `vm_map_entry`, page faults
- Zone allocator (Mach-style fixed-size zones for kernel objects)
- Higher-half direct map (HHDM) usage via Limine response

Key new types: `vm_page_t`, `vm_map_entry_t`, `pmap_t`, `zone_t`

## Phase 3 — Scheduler and Threads

- Full x86\_64 thread context save/restore
- Simple round-robin scheduler
- Kernel threads runnable
- Idle thread per CPU
- Per-CPU data structure (`cpu_data_t`)
- `kern/clock.h`: monotonic tick from PIT or HPET

## Phase 4 — IPC

- Port namespace per task (`ipc_space_t`)
- `mach_msg()` kernel trap: send and receive paths
- Copy-in / copy-out of message bodies
- Port rights: SEND, RECEIVE, SEND\_ONCE
- Simple port sets
- Bootstrap port

Key functions: `mach_msg_send()`, `mach_msg_receive()`, `ipc_kmsg_get()`

## Phase 5 — Syscall Interface and First User Task

- x86\_64 `syscall`/`sysret` entry via LSTAR MSR
- Mach trap table matching XNU negative syscall numbering

## Phase 6 — Darwin ABI Compatibility (current)

- Mach-O 64-bit binary loader (`LC_SEGMENT_64`, `LC_UNIXTHREAD`)
- BSD personality layer over Mach (task = BSD process)
- POSIX syscall shim routed through BSD layer
- `dyld` bootstrap handoff
- Running simple static Darwin/macOS binaries

## Phase 7 and Beyond

- Second HAL: aarch64 (UEFI via Limine)
- SMP: per-CPU scheduler, spinlocks, IPI
- Mach exception handling (`EXC_BAD_ACCESS`, `EXC_BAD_INSTRUCTION`)
- IOKit stub layer
- Network stack
- Full userland with `libSystem` shim
