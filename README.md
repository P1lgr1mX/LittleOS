# AetherOS (LittleOS)

<div align="center">
  <img src="https://www.wallper.app/wallpaper/anime/popout-manga-88467e27-65d8-47e9-9951-840d49ac3804" alt="AetherOS">
</div>

A bare-metal 32-bit x86 operating system featuring a higher-half monolithic kernel, paging and virtual memory management, interrupt routing, segmentation, preemptive-ready hardware abstraction, a bitmap heap allocator, and an isolated Ring 3 userland shell environment.

## Architectural Overview

AetherOS is designed from first principles using:
- **C (Freestanding GNU C Dialect)**
- **x86 IA-32 Assembly (NASM)**
- **GNU Toolchain (`gcc`, `ld`, `objcopy`, `genisoimage`)**
- **QEMU Emulation Platform**

### Kernel Subsystems and Capabilities:
- **Multiboot Bootstrap**: Multiboot-compliant kernel entry point supporting GRUB stage2 El Torito bootloader.
- **Two-Tier Paging MMU**: Higher-Half kernel architecture linked at virtual address `0xC0100000` (3GB base offset) with identity bootstrap mapping.
- **Segmentation (GDT) & Task State Segment (TSS)**: Flat memory model descriptors for Ring 0 Kernel and Ring 3 Userland code/data segments, alongside TSS hardware stack privilege transition (`esp0`).
- **Interrupts and Exceptions**: 256-entry Interrupt Descriptor Table (IDT), 32 CPU hardware exception handlers, and cascaded Dual 8259 Programmable Interrupt Controller (PIC) routing (vectors 32–47).
- **Driver Layer**:
  - Memory-mapped VGA text-mode framebuffer (80x25) with hardware cursor synchronization, color palettes, and terminal scrolling.
  - 16550A UART Serial Driver (COM1, 115200 8N1, FIFO buffer, IRQ 4).
  - PS/2 Keyboard Driver with Scancode Set 1 decoding, Shift state handling, and a circular FIFO ring buffer.
- **Dynamic Memory Allocation**: Kernel Bitmap Heap Allocator (`kmalloc`, `kcalloc`, `krealloc`, `kfree`) with First-Fit search, block headers, and corruption detection sentinels.
- **System Call Subsystem**: Software interrupt gate `INT 0x80` with `pt_regs` register context preservation (`SYS_EXIT`, `SYS_FORK`, `SYS_READ`, `SYS_WRITE`, `SYS_CLEAR`).
- **Userland Runtime & Shell (Ring 3)**:
  - Freestanding userland C runtime library (`libc`) providing string routines and formatted output (`printf`, `puts`, `gets`).
  - Interactive User Shell running in Ring 3 with command dispatching (`help`, `clear`, `echo`, `about`).

## Project Layout

```text
AetherOS
├── arch
│   └── x86
│       ├── boot
│       │   ├── grub
│       │   │   ├── menu.lst
│       │   │   └── stage2_eltorito
│       │   └── loader.s
│       ├── cpu
│       │   ├── gdt.c
│       │   ├── idt.c
│       │   ├── idt.s
│       │   ├── io.s
│       │   ├── isr.c
│       │   └── isr.s
│       └── mmu
│           ├── paging.c
│           └── paging.h
├── boot
│   └── grub
│       ├── menu.lst
│       └── stage2_eltorito
├── drivers
│   ├── framebuffer.c
│   ├── keyboard.c
│   ├── pic.c
│   └── serial.c
├── include
│   ├── arch
│   │   └── x86
│   │       ├── gdt.h
│   │       ├── idt.h
│   │       ├── io.h
│   │       └── isr.h
│   ├── drivers
│   │   ├── framebuffer.h
│   │   ├── keyboard.h
│   │   ├── pic.h
│   │   └── serial.h
│   ├── kernel
│   │   ├── kheap.h
│   │   ├── multiboot.h
│   │   └── syscall.h
│   └── types.h
├── kernel
│   ├── kheap.c
│   ├── kheap.s
│   ├── kmain.c
│   ├── syscall.c
│   └── syscall.s
├── userland
│   ├── libc
│   │   ├── include
│   │   │   ├── stdio.h
│   │   │   ├── string.h
│   │   │   └── unistd.h
│   │   └── src
│   │       ├── entry.s
│   │       ├── stdio.c
│   │       ├── string.c
│   │       └── syscall.s
│   ├── shell
│   │   ├── commands.c
│   │   ├── commands.h
│   │   └── shell.c
│   └── user.ld
├── modules
│   └── program.s
├── link.ld
├── Makefile
└── README.md
```

## Compilation and Execution

### Prerequisites
Ensure the following packages are installed on your host system:
```bash
sudo apt-get install build-essential nasm genisoimage qemu-system-x86
```

### Build Targets
To compile the kernel, userland shell, and generate a bootable ISO image:
```bash
make clean
make
```

### Running in Emulation
To run the bootable ISO in QEMU with serial console routed to stdio:
```bash
make run
```

To boot directly into the kernel ELF binary:
```bash
make run-kernel
```
