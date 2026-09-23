BITS 32
[ORG 0x00000000]

start:
    ; Gọi ngắt hệ thống int 0x80: SYS_WRITE (4)
    mov eax, 4          ; SYS_WRITE
    mov ebx, 1          ; stdout
    mov ecx, msg        ; địa chỉ chuỗi
    mov edx, msg_len    ; độ dài chuỗi
    int 0x80

    ; Lặp vô tận sau khi hoàn thành syscall
    jmp $

msg: db "[User Mode Ring 3] Syscall 0x80 SYS_WRITE called successfully!", 0x0A, 0x0D, 0
msg_len equ $ - msg - 1