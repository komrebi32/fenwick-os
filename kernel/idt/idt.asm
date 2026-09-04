section .text
bits 64

global idt_load_asm
global isr_stub_0
global isr_stub_1
global isr_stub_2
global isr_stub_3
global isr_stub_4
global isr_stub_5
global isr_stub_6
global isr_stub_7
global isr_stub_8
global isr_stub_9
global isr_stub_10
global isr_stub_11
global isr_stub_12
global isr_stub_13
global isr_stub_14
global isr_stub_15
global isr_stub_16
global isr_stub_17
global isr_stub_18
global isr_stub_19
global isr_stub_20
global isr_stub_21
global isr_stub_22
global isr_stub_23
global isr_stub_24
global isr_stub_25
global isr_stub_26
global isr_stub_27
global isr_stub_28
global isr_stub_29
global isr_stub_30
global isr_stub_31
global irq_stub_0
global irq_stub_1
global irq_stub_2
global irq_stub_3
global irq_stub_4
global irq_stub_5
global irq_stub_6
global irq_stub_7
global irq_stub_8
global irq_stub_9
global irq_stub_10
global irq_stub_11
global irq_stub_12
global irq_stub_13
global irq_stub_14
global irq_stub_15

extern isr_handler
extern irq_handler

idt_load_asm:
    mov rax, rdi
    lidt [rax]
    ret

%macro ISR_NO_ERR 1
isr_stub_%1:
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
isr_stub_%1:
    push %1
    jmp isr_common
%endmacro

%macro IRQ 2
irq_stub_%1:
    push 0
    push %2
    jmp irq_common
%endmacro

ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_ERR 8
ISR_NO_ERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_ERR 17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20
ISR_NO_ERR 21
ISR_NO_ERR 22
ISR_NO_ERR 23
ISR_NO_ERR 24
ISR_NO_ERR 25
ISR_NO_ERR 26
ISR_NO_ERR 27
ISR_NO_ERR 28
ISR_NO_ERR 29
ISR_NO_ERR 30
ISR_NO_ERR 31

IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

isr_common:
    cld
    mov rdi, [rsp + 8]
    mov rsi, [rsp + 16]
    call isr_handler
    add rsp, 16
    iretq

irq_common:
    cld
    mov rdi, [rsp + 8]
    call irq_handler
    add rsp, 16
    iretq
