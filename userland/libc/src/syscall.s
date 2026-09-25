global sys_exit
global sys_read
global sys_write
global sys_clear

section .text
align 4

sys_exit:
    push ebx
    mov eax, 1          ; SYS_EXIT
    mov ebx, [esp + 8]  ; exit code
    int 0x80
    pop ebx
    ret

sys_read:
    push ebx
    mov eax, 3          ; SYS_READ
    mov ebx, [esp + 8]  ; fd
    mov ecx, [esp + 12] ; buf
    mov edx, [esp + 16] ; count
    int 0x80
    pop ebx
    ret

sys_write:
    push ebx
    mov eax, 4          ; SYS_WRITE
    mov ebx, [esp + 8]  ; fd
    mov ecx, [esp + 12] ; buf
    mov edx, [esp + 16] ; count
    int 0x80
    pop ebx
    ret

sys_clear:
    mov eax, 5          ; SYS_CLEAR
    int 0x80
    ret
