BITS 32
[ORG 0x00000000]

start:
    ; 1. Display banner indicating active canonical buffer mode in Ring 3
    mov eax, 4          ; SYS_WRITE
    mov ebx, 1          ; stdout
    mov ecx, msg_banner
    mov edx, msg_banner_len
    int 0x80

user_loop:
    ; Display userland prompt
    mov eax, 4          ; SYS_WRITE
    mov ebx, 1          ; stdout
    mov ecx, prompt
    mov edx, prompt_len
    int 0x80

    ; 2. Invoke SYS_READ to receive a line-buffered input string
    ; The kernel accumulates keystrokes until Enter (\n) or null terminator
    mov eax, 3          ; SYS_READ
    mov ebx, 0          ; stdin
    mov ecx, user_buffer ; user space destination buffer
    mov edx, 128        ; maximum buffer capacity (128 bytes)
    int 0x80

    ; eax contains byte count of received input
    ; If empty or error (eax <= 0), repeat prompt
    cmp eax, 0
    jle user_loop

    ; Store length of received string
    mov [str_len], eax

    ; 3. Print receipt message prefix
    mov eax, 4
    mov ebx, 1
    mov ecx, msg_recv
    mov edx, msg_recv_len
    int 0x80

    ; Output the buffer received from SYS_READ
    mov eax, 4
    mov ebx, 1
    mov ecx, user_buffer
    mov edx, [str_len]
    int 0x80

    ; Output closing quote and newline
    mov eax, 4
    mov ebx, 1
    mov ecx, msg_end
    mov edx, msg_end_len
    int 0x80

    ; Loop to await subsequent user input
    jmp user_loop

section .data
msg_banner:
    db 0x0A, "=================================================================", 0x0A
    db " [User Ring 3] Canonical Buffer Input Mode is ACTIVE!", 0x0A
    db " Type a full sentence and press ENTER (buffered until \n or \0):", 0x0A
    db "=================================================================", 0x0A, 0
msg_banner_len equ $ - msg_banner

prompt:
    db "UserRing3> ", 0
prompt_len equ $ - prompt

msg_recv:
    db "[User Ring 3 Received Buffer]: ", 0x22, 0
msg_recv_len equ $ - msg_recv

msg_end:
    db 0x22, 0x0A, 0x0D, 0
msg_end_len equ $ - msg_end

str_len:
    dd 0

section .bss
user_buffer:
    resb 128