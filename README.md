# LittleOS

<div align="center">
  <img src="https://wall.alphacoders.com/big.php?i=1406676" alt="LittleOS">
</div>

A bare-metal operating system project focused on low-level x86 kernel development, bootloader setup, VGA framebuffer output, and hardware interrupt handling. This project is designed as an educational OS development exercise and explores how a minimal kernel boots and interacts with system hardware directly.

## Overview

LittleOS is a small custom operating system built from the ground up using:
- C
- x86 Assembly
- GNU toolchain
- QEMU for emulation

The project currently includes:
- a custom bootloader
- kernel entry point
- VGA text-mode framebuffer driver
- serial port driver
- GDT (Global Descriptor Table) setup & reloading
- IDT (Interrupt Descriptor Table) with 48 interrupt gates
- 8259 PIC remapping and hardware IRQ handling
- PS/2 Keyboard driver with Scancode Set 1 decoding
- GRUB-compatible ISO generation
- basic kernel startup and output routines

## Features

- Bare-metal x86 kernel startup
- Bootloader entry via `loader`
- VGA text mode framebuffer rendering
- Hardware cursor synchronization
- Serial COM1 communication
- Auto-scroll support for console output
- Basic color support for terminal output
- GDT segmentation setup (code and data descriptors)
- IDT with CPU exceptions & PIC hardware interrupt routing
- Interactive PS/2 keyboard typing with screen and serial echo
- ISO image generation for booting in QEMU
- Minimal kernel execution environment without libc

## Project Structure

```text
LittleOS
├
├── boot
│   ├── grub
│   │   ├── menu.lst
│   │   └── stage2_eltorito
│   └── loader.s
├── docs
│   └── example.c
├── drivers
│   ├── framebuffer.c
│   ├── framebuffer.h
│   ├── gdt.c
│   ├── gdt.h
│   ├── idt.c
│   ├── idt.h
│   ├── idt.s
│   ├── io.h
│   ├── io.s
│   ├── isr.c
│   ├── isr.h
│   ├── isr.s
│   ├── keyboard.c
│   ├── keyboard.h
│   ├── pic.c
│   ├── pic.h
│   ├── serial.c
│   └── serial.h
├── kernel
│   ├── kmain.c
│   ├── multiboot.h
│   ├── paging.c
│   └── paging.h
├── link.ld
├── Makefile
├── modules
│   └── program.s
└── README.md

```