section .text
global serial_putc

serial_putc:
    push rdx
    push rax
.wait:
    mov dx, 0x3FD
    in al, dx
    test al, 0x20
    jz .wait
    pop rax
    pop rdx
    mov dx, 0x3F8
    out dx, al
    ret
