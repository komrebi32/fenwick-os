section .text
global serial_putc

serial_putc:
    mov dx, 0x3F8
    mov al, dil
    out dx, al
    ret
