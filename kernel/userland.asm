section .text
bits 64

global userland_enter

userland_enter:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rsp, rsi
    push 0x23
    push rsi
    pushfq
    push 0x1B
    push rdi
    iretq
