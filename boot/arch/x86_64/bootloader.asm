bits 32

section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .boot
global start
extern krnl_main

MBALIGN     equ 1 << 0
MEMINFO     equ 1 << 1
MAGIC       equ 0x1BADB002
FLAGS       equ MBALIGN | MEMINFO
CHECKSUM    equ -(MAGIC + FLAGS)

start:
    cli
    mov esp, stack_top

    call check_cpuid
    call check_long_mode
    call setup_paging

    mov eax, cr4
    or eax, (1 << 5) | (1 << 7)
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8) | (1 << 11)
    wrmsr

    lgdt [gdt64.pointer]

    mov eax, cr0
    or eax, (1 << 31) | (1 << 0)
    mov cr0, eax

    jmp 0x08:long_mode_entry

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, (1 << 21)
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    hlt
    jmp .no_cpuid

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    jz .no_long_mode
    ret
.no_long_mode:
    hlt
    jmp .no_long_mode

setup_paging:
    mov eax, pdpt_table
    or eax, 0b11
    mov [pml4_table], eax

    mov eax, pd_table
    or eax, 0b11
    mov [pdpt_table], eax

    mov ecx, 0
.map_loop:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [pd_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_loop

    mov eax, pml4_table
    mov cr3, eax
    ret

section .text64
bits 64
long_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    call krnl_main

.halt:
    cli
    hlt
    jmp .halt

section .rodata
gdt64:
    dq 0
.code_selector equ 0x08
    dq 0x00AF9B000000FFFF
.data_selector equ 0x10
    dq 0x00AF93000000FFFF
.pointer:
    dw .pointer - gdt64 - 1
    dq gdt64

section .bss
align 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096
stack_bottom:
    resb 16384
stack_top:
