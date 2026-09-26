global _start
extern main
extern sys_exit

section .text.entry
align 4

_start:
    ; Call userland main entry point in C
    call main

    ; Invoke sys_exit syscall with return code from eax
    push eax
    call sys_exit

    ; Guard loop in case sys_exit returns
.halt:
    hlt
    jmp .halt
