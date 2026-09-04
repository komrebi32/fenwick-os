section .text
global gdt_load

gdt_load:
    mov rax, rdi
    lgdt [rax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
