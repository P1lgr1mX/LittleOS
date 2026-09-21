# LittleOS

<div align="center">
  <img src="https://images.unsplash.com/photo-1518770660439-4636190af475?auto=format&fit=crop&w=1200&q=80" alt="LittleOS banner" width="1000">
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
- ISO image generation for booting in QEMU
- Minimal kernel execution environment without libc

## Project Structure

```text
LittleOS/
├── boot/
│   ├── grub/
│   │   ├── menu.lst
│   │   └── stage2_eltorito
│   └── loader.s
├── docs/
│   └── example.c
├── drivers/
│   ├── framebuffer.c
│   ├── framebuffer.h
│   ├── io.h
│   ├── io.s
│   ├── isr.h
│   ├── isr.s
│   ├── serial.c
│   └── serial.h
├── kernel/
│   └── kmain.c
├── .gitignore
├── Makefile
├── link.ld
└── README.md