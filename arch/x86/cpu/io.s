global outb 

section .text
outb:
    mov dx, [esp + 4]    ; port: 16-bit target I/O port
    mov al, [esp + 8]    ; value: 8-bit data byte to send
    out dx, al           ; output byte from AL to port in DX
    ret

global inb 
inb: 
    mov dx, [esp + 4]    ; port: 16-bit source I/O port
    in al, dx            ; input byte from port in DX to AL
    ret

global enable_interrupts
enable_interrupts:
    sti                  ; enable CPU hardware interrupts (set IF)
    ret

global disable_interrupts
disable_interrupts:
    cli                  ; disable CPU hardware interrupts (clear IF)
    ret
