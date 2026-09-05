section .text
bits 64

global syscall_entry
global userland_entry

extern syscall_dispatcher

syscall_entry:
    push rcx
    push r11
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov r15, rax
    mov r14, rdi
    mov r13, rsi
    mov r12, rdx
    mov rbx, r10
    push rbx
    mov rbx, r8
    push rbx
    mov rbx, r9
    push rbx
    push r14
    push r13
    push r12
    push rbp
    push rbx
    push r15

    mov rdi, rax
    mov rsi, r14
    mov rdx, r13
    mov rcx, r12
    mov r8, [rsp + 8*5]
    mov r9, [rsp + 8*6]

    call syscall_dispatcher

    add rsp, 64
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    pop r11
    pop rcx
    sysretq

userland_entry:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rsp, rdi
    push 0x23
    push rdi
    pushfq
    push 0x1B
    push rsi
    iretq
