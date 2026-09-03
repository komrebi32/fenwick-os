bits 32

section .boot
global start
extern krnl_main

MBALIGN     equ 1 << 0
MEMINFO     equ 1 << 1
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

align 4
multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

start:
    mov esp, stack_top

    call check_cpuid
    call check_long_mode
    call setup_paging

    ; PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Activar modo largo
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Activar la paginacion
    mov eax, cr0
    or eax, 1 << 31 | 1 << 0
    mov cr0, eax

    lgdt [gdt64.pointer]
    jmp gdt64.code_selector:long_mode_entry

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
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

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    hlt

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

bits 64
long_mode_entry:
    mov ax, gdt64.data_selector
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

section .bss
align 4096
pml4_table: resb 4096
pdpt_table: resb 4096
pd_table:   resb 4096

stack_bottom:
    resb 16384
stack_top:

section .rodata
gdt64:
    dq 0
.code_selector: equ $ - gdt64
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
.data_selector: equ $ - gdt64
    dq (1 << 44) | (1 << 47) | (1 << 41)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64