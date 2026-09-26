; =============================================================================
; arch/x86/boot/loader.s: Kernel Entry Point and Bootstrap Higher-Half Paging
; =============================================================================

global _start
global loader
global load_gdt
global load_page_directory
global enable_paging
global boot_page_directory
global boot_page_table1
global load_tss
global enter_user_mode
global kernel_stack
global KERNEL_STACK_SIZE
extern kmain

; Multiboot 1 specification constants
MAGIC_NUMBER        equ 0x1BADB002
ALIGN_MODULES       equ 0x00000001              ; Request bootloader to align loaded modules on 4KB page boundaries
FLAGS               equ ALIGN_MODULES
CHECKSUM            equ -(MAGIC_NUMBER + FLAGS) ; Satisfies the requirement: magic + flags + checksum == 0

KERNEL_STACK_SIZE   equ 4096                    ; Kernel bootstrap stack allocation (4KB)
KERNEL_VIRTUAL_BASE equ 0xC0000000              ; Virtual base address of the higher-half kernel (3GB)

; =============================================================================
; Section .multiboot: GRUB header within the first 8KB of the ELF executable
; =============================================================================
section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

; =============================================================================
; Section .boot: Executes at physical 1MB (0x00100000) prior to paging enablement
; =============================================================================
section .boot
_start:
loader:
    ; 1. Load the physical address of the bootstrap page directory into CR3.
    ;    Since boot_page_directory is linked in virtual space (0xC010xxxx),
    ;    subtract KERNEL_VIRTUAL_BASE to obtain its physical address (0x0010xxxx).
    mov eax, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, eax

    ; 2. Enable paging: Set bit 31 (PG - Paging Enable) in control register CR0.
    ;    Due to identity mapping of the first 4MB (directory entry 0), instruction
    ;    fetching continues seamlessly at the current physical address.
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    ; 3. Execute an absolute jump into higher-half virtual address space.
    ;    The label `higher_half` resides in .text at virtual address >= 0xC0100000.
    lea eax, [higher_half]
    jmp eax

; =============================================================================
; Section .text: Primary kernel code running in higher-half virtual space (>= 0xC0100000)
; =============================================================================
section .text
higher_half:
    ; 4. Initialize the kernel stack pointer (ESP) to point to the top of the stack
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    ; 5. Push the Multiboot information structure pointer (EBX) per cdecl calling convention
    push ebx

    ; 6. Transfer execution to kmain in C (located at virtual address 0xC010xxxx)
    call kmain

; Infinite halt loop in case kmain returns
.loop:
    hlt
    jmp .loop

; Load the Global Descriptor Table Register (GDTR) and reload segment registers
load_gdt:
    mov eax, [esp + 4]              ; Argument: pointer to struct gdt_ptr
    lgdt [eax]                      ; Load GDTR

    mov ax, 0x10                    ; Kernel Data Segment selector (offset 0x10 in GDT)
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp 0x08:.flush_cs              ; Far jump to flush instruction pipeline and reload CS (0x08)

.flush_cs:
    ret

; Load page directory base physical address into CR3
load_page_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

; Enable MMU paging by setting bit 31 (PG) in CR0
enable_paging:
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
    ret

; Load Task Register (TR): load_tss(uint16_t selector)
load_tss:
    mov ax, [esp + 4]
    ltr ax
    ret

; Transition to Ring 3 (User Mode) via iret frame: enter_user_mode(eip, esp)
enter_user_mode:
    mov eax, [esp + 4]              ; Target EIP of userland process
    mov ebx, [esp + 8]              ; Target ESP (user stack top)

    ; Load User Data Segment selector: 0x20 | 3 = 0x23 (RPL=3)
    mov cx, 0x23
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx

    ; Construct interrupt return frame for privilege transition:
    ; Stack order expected by CPU iret: EIP -> CS -> EFLAGS -> ESP -> SS
    push dword 0x23                 ; SS: User Data Segment selector (RPL=3)
    push ebx                        ; ESP: User stack pointer (e.g. 0xBFFFFFFB)
    push dword 0x202                ; EFLAGS: Bit 1 reserved (1), Bit 9 (IF=1: interrupts enabled)
    push dword 0x1B                 ; CS: User Code Segment selector (0x18 | 3, RPL=3)
    push eax                        ; EIP: User program entry point (e.g. 0x00000000)

    iret                            ; CPU pops frame and lowers privilege level to Ring 3

; =============================================================================
; Section .data: Statically compiled bootstrap paging structures
; =============================================================================
section .data
align 4096
boot_page_table1:
    ; Map 1024 page table entries (4KB each) to cover the initial 4MB of physical RAM
    ; Flags 3 (0b011): Present = 1, Read/Write = 1, Supervisor = 0
    %assign i 0
    %rep 1024
        dd (i * 4096) | 3
        %assign i i+1
    %endrep

align 4096
boot_page_directory:
    ; Entry 0 (virtual 0MB - 4MB):
    ; Identity map physical 0MB - 4MB to prevent execution faults during paging transition
    dd (boot_page_table1 - KERNEL_VIRTUAL_BASE) + 3
    times (768 - 1) dd 0

    ; Entry 768 (virtual 3GB - 3GB+4MB, offset 0x300):
    ; Higher-half mapping to physical 0MB - 4MB where the kernel binary resides
    dd (boot_page_table1 - KERNEL_VIRTUAL_BASE) + 3
    times (1024 - 768 - 1) dd 0

; =============================================================================
; Section .bss: Kernel bootstrap stack allocation
; =============================================================================
section .bss
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE
