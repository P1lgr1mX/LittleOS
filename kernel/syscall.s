%define ENOSYS 38

global Int128Handler
extern do_syscall_in_C

section .text
align 4

Int128Handler:
    ; When software interrupt (int 0x80) triggers from Ring 3, CPU hardware automatically pushes:
    ;   [esp + 16]: ss (Ring 3)
    ;   [esp + 12]: esp (Ring 3)
    ;   [esp + 8]:  eflags
    ;   [esp + 4]:  cs (0x1B)
    ;   [esp + 0]:  eip

    ; Push registers matching the layout of struct pt_regs in include/kernel/syscall.h:
    push eax            ; [esp + 44]: Syscall number from User Mode
    push gs             ; [esp + 40]
    push fs             ; [esp + 36]
    push es             ; [esp + 32]
    push ds             ; [esp + 28]
    push dword -ENOSYS  ; [esp + 24]: orig_eax (default error code if unhandled)
    push ebp            ; [esp + 20]
    push edi            ; [esp + 16]
    push esi            ; [esp + 12]
    push edx            ; [esp + 8]
    push ecx            ; [esp + 4]
    push ebx            ; [esp + 0]

    ; Switch DS and ES to Kernel Data Segment (0x10) for safe kernel memory addressing
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    cld                 ; Clear direction flag (ensure string instructions increment)

    ; Pass pointer to struct pt_regs to C dispatcher
    push esp
    call do_syscall_in_C
    add esp, 4          ; Clean up argument pointer on stack

    ; Restore general-purpose registers
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp

    ; Discard orig_eax slot from stack
    add esp, 4

    ; Restore user segment registers
    pop ds
    pop es
    pop fs
    pop gs

    ; Restore eax (containing syscall return code stored in regs->eax)
    pop eax

    ; Return to User Mode (Ring 3); CPU hardware pops eip, cs, eflags, esp, ss
    iret
