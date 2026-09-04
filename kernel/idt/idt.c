#include "idt.h"
#include "panic.h"
#include "kbd.h"
#include <libk.h>

static struct idt_entry idt_entries[IDT_ENTRIES];
static struct idt_ptr idt_descriptor;

extern void idt_load_asm(struct idt_ptr *ptr);
extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_20(void);
extern void isr_stub_21(void);
extern void isr_stub_22(void);
extern void isr_stub_23(void);
extern void isr_stub_24(void);
extern void isr_stub_25(void);
extern void isr_stub_26(void);
extern void isr_stub_27(void);
extern void isr_stub_28(void);
extern void isr_stub_29(void);
extern void isr_stub_30(void);
extern void isr_stub_31(void);
extern void irq_stub_0(void);
extern void irq_stub_1(void);
extern void irq_stub_2(void);
extern void irq_stub_3(void);
extern void irq_stub_4(void);
extern void irq_stub_5(void);
extern void irq_stub_6(void);
extern void irq_stub_7(void);
extern void irq_stub_8(void);
extern void irq_stub_9(void);
extern void irq_stub_10(void);
extern void irq_stub_11(void);
extern void irq_stub_12(void);
extern void irq_stub_13(void);
extern void irq_stub_14(void);
extern void irq_stub_15(void);

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void io_wait(void) {
    outb(0x80, 0);
}

static void pic_remap(void) {
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait();
    outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();
    outb(0x21, 0x00); io_wait();
    outb(0xA1, 0x00); io_wait();
}

static void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

static void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

volatile uint64_t timer_ticks = 0;

uint64_t timer_get_ticks(void) {
    return timer_ticks;
}

void timer_wait(uint64_t ms) {
    uint64_t target = timer_ticks + ms / 10;
    while (timer_ticks < target) {
        asm volatile("hlt");
    }
}

static const char *exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

void isr_handler(uint64_t int_no, uint64_t err_code) {
    switch (int_no) {
        case 8:
            panic_handler("Double Fault - Critical error detected", int_no, err_code);
            break;
        case 13:
            panic_handler("General Protection Fault - Memory protection violated", int_no, err_code);
            break;
        case 14:
            panic_handler("Page Fault - Invalid memory access", int_no, err_code);
            break;
        case 18:
            panic_handler("Machine Check - Hardware failure", int_no, err_code);
            break;
        case 0:
            panic_handler("Division By Zero", int_no, err_code);
            break;
        case 6:
            panic_handler("Invalid Opcode", int_no, err_code);
            break;
        case 12:
            panic_handler("Stack-Segment Fault", int_no, err_code);
            break;
        default:
            panic_handler(exception_messages[int_no], int_no, err_code);
            break;
    }
}

void irq_handler(uint64_t int_no) {
    kset_color(0x07);
    switch (int_no) {
        case 32:
            timer_ticks++;
            pic_send_eoi(0);
            return;
        case 33:
            kbd_handler();
            pic_send_eoi(1);
            return;
        default:
            pic_send_eoi(int_no - 32);
            return;
    }
}

void idt_install(void) {
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_entries[i].offset_low = 0;
        idt_entries[i].selector = 0;
        idt_entries[i].ist = 0;
        idt_entries[i].flags = 0;
        idt_entries[i].offset_mid = 0;
        idt_entries[i].offset_high = 0;
        idt_entries[i].reserved = 0;
    }

    uint8_t flags = 0x8E;

    idt_entries[0].offset_low  = (uint16_t)((uint64_t)isr_stub_0  & 0xFFFF);
    idt_entries[0].selector    = 0x08;
    idt_entries[0].flags       = flags;
    idt_entries[0].offset_mid  = (uint16_t)(((uint64_t)isr_stub_0  >> 16) & 0xFFFF);
    idt_entries[0].offset_high = (uint32_t)(((uint64_t)isr_stub_0  >> 32) & 0xFFFFFFFF);

    idt_entries[1].offset_low  = (uint16_t)((uint64_t)isr_stub_1  & 0xFFFF);
    idt_entries[1].selector    = 0x08;
    idt_entries[1].flags       = flags;
    idt_entries[1].offset_mid  = (uint16_t)(((uint64_t)isr_stub_1  >> 16) & 0xFFFF);
    idt_entries[1].offset_high = (uint32_t)(((uint64_t)isr_stub_1  >> 32) & 0xFFFFFFFF);

    idt_entries[2].offset_low  = (uint16_t)((uint64_t)isr_stub_2  & 0xFFFF);
    idt_entries[2].selector    = 0x08;
    idt_entries[2].flags       = flags;
    idt_entries[2].offset_mid  = (uint16_t)(((uint64_t)isr_stub_2  >> 16) & 0xFFFF);
    idt_entries[2].offset_high = (uint32_t)(((uint64_t)isr_stub_2  >> 32) & 0xFFFFFFFF);

    idt_entries[3].offset_low  = (uint16_t)((uint64_t)isr_stub_3  & 0xFFFF);
    idt_entries[3].selector    = 0x08;
    idt_entries[3].flags       = flags;
    idt_entries[3].offset_mid  = (uint16_t)(((uint64_t)isr_stub_3  >> 16) & 0xFFFF);
    idt_entries[3].offset_high = (uint32_t)(((uint64_t)isr_stub_3  >> 32) & 0xFFFFFFFF);

    idt_entries[4].offset_low  = (uint16_t)((uint64_t)isr_stub_4  & 0xFFFF);
    idt_entries[4].selector    = 0x08;
    idt_entries[4].flags       = flags;
    idt_entries[4].offset_mid  = (uint16_t)(((uint64_t)isr_stub_4  >> 16) & 0xFFFF);
    idt_entries[4].offset_high = (uint32_t)(((uint64_t)isr_stub_4  >> 32) & 0xFFFFFFFF);

    idt_entries[5].offset_low  = (uint16_t)((uint64_t)isr_stub_5  & 0xFFFF);
    idt_entries[5].selector    = 0x08;
    idt_entries[5].flags       = flags;
    idt_entries[5].offset_mid  = (uint16_t)(((uint64_t)isr_stub_5  >> 16) & 0xFFFF);
    idt_entries[5].offset_high = (uint32_t)(((uint64_t)isr_stub_5  >> 32) & 0xFFFFFFFF);

    idt_entries[6].offset_low  = (uint16_t)((uint64_t)isr_stub_6  & 0xFFFF);
    idt_entries[6].selector    = 0x08;
    idt_entries[6].flags       = flags;
    idt_entries[6].offset_mid  = (uint16_t)(((uint64_t)isr_stub_6  >> 16) & 0xFFFF);
    idt_entries[6].offset_high = (uint32_t)(((uint64_t)isr_stub_6  >> 32) & 0xFFFFFFFF);

    idt_entries[7].offset_low  = (uint16_t)((uint64_t)isr_stub_7  & 0xFFFF);
    idt_entries[7].selector    = 0x08;
    idt_entries[7].flags       = flags;
    idt_entries[7].offset_mid  = (uint16_t)(((uint64_t)isr_stub_7  >> 16) & 0xFFFF);
    idt_entries[7].offset_high = (uint32_t)(((uint64_t)isr_stub_7  >> 32) & 0xFFFFFFFF);

    idt_entries[8].offset_low  = (uint16_t)((uint64_t)isr_stub_8  & 0xFFFF);
    idt_entries[8].selector    = 0x08;
    idt_entries[8].flags       = flags;
    idt_entries[8].offset_mid  = (uint16_t)(((uint64_t)isr_stub_8  >> 16) & 0xFFFF);
    idt_entries[8].offset_high = (uint32_t)(((uint64_t)isr_stub_8  >> 32) & 0xFFFFFFFF);

    idt_entries[10].offset_low  = (uint16_t)((uint64_t)isr_stub_10  & 0xFFFF);
    idt_entries[10].selector    = 0x08;
    idt_entries[10].flags       = flags;
    idt_entries[10].offset_mid  = (uint16_t)(((uint64_t)isr_stub_10  >> 16) & 0xFFFF);
    idt_entries[10].offset_high = (uint32_t)(((uint64_t)isr_stub_10  >> 32) & 0xFFFFFFFF);

    idt_entries[11].offset_low  = (uint16_t)((uint64_t)isr_stub_11  & 0xFFFF);
    idt_entries[11].selector    = 0x08;
    idt_entries[11].flags       = flags;
    idt_entries[11].offset_mid  = (uint16_t)(((uint64_t)isr_stub_11  >> 16) & 0xFFFF);
    idt_entries[11].offset_high = (uint32_t)(((uint64_t)isr_stub_11  >> 32) & 0xFFFFFFFF);

    idt_entries[12].offset_low  = (uint16_t)((uint64_t)isr_stub_12  & 0xFFFF);
    idt_entries[12].selector    = 0x08;
    idt_entries[12].flags       = flags;
    idt_entries[12].offset_mid  = (uint16_t)(((uint64_t)isr_stub_12  >> 16) & 0xFFFF);
    idt_entries[12].offset_high = (uint32_t)(((uint64_t)isr_stub_12  >> 32) & 0xFFFFFFFF);

    idt_entries[13].offset_low  = (uint16_t)((uint64_t)isr_stub_13  & 0xFFFF);
    idt_entries[13].selector    = 0x08;
    idt_entries[13].flags       = flags;
    idt_entries[13].offset_mid  = (uint16_t)(((uint64_t)isr_stub_13  >> 16) & 0xFFFF);
    idt_entries[13].offset_high = (uint32_t)(((uint64_t)isr_stub_13  >> 32) & 0xFFFFFFFF);

    idt_entries[14].offset_low  = (uint16_t)((uint64_t)isr_stub_14  & 0xFFFF);
    idt_entries[14].selector    = 0x08;
    idt_entries[14].flags       = flags;
    idt_entries[14].offset_mid  = (uint16_t)(((uint64_t)isr_stub_14  >> 16) & 0xFFFF);
    idt_entries[14].offset_high = (uint32_t)(((uint64_t)isr_stub_14  >> 32) & 0xFFFFFFFF);

    for (int i = 15; i < 32; i++) {
        void (*stub)(void) = (void(*)(void))0;
        switch (i) {
            case 15: stub = isr_stub_15; break;
            case 16: stub = isr_stub_16; break;
            case 17: stub = isr_stub_17; break;
            case 18: stub = isr_stub_18; break;
            case 19: stub = isr_stub_19; break;
            case 20: stub = isr_stub_20; break;
            case 21: stub = isr_stub_21; break;
            case 22: stub = isr_stub_22; break;
            case 23: stub = isr_stub_23; break;
            case 24: stub = isr_stub_24; break;
            case 25: stub = isr_stub_25; break;
            case 26: stub = isr_stub_26; break;
            case 27: stub = isr_stub_27; break;
            case 28: stub = isr_stub_28; break;
            case 29: stub = isr_stub_29; break;
            case 30: stub = isr_stub_30; break;
            case 31: stub = isr_stub_31; break;
        }
        idt_entries[i].offset_low  = (uint16_t)((uint64_t)stub & 0xFFFF);
        idt_entries[i].selector    = 0x08;
        idt_entries[i].flags       = flags;
        idt_entries[i].offset_mid  = (uint16_t)(((uint64_t)stub >> 16) & 0xFFFF);
        idt_entries[i].offset_high = (uint32_t)(((uint64_t)stub >> 32) & 0xFFFFFFFF);
    }

    void *irq_stubs[16] = {
        irq_stub_0, irq_stub_1, irq_stub_2, irq_stub_3,
        irq_stub_4, irq_stub_5, irq_stub_6, irq_stub_7,
        irq_stub_8, irq_stub_9, irq_stub_10, irq_stub_11,
        irq_stub_12, irq_stub_13, irq_stub_14, irq_stub_15
    };

    for (int i = 0; i < 16; i++) {
        idt_entries[32 + i].offset_low  = (uint16_t)((uint64_t)irq_stubs[i] & 0xFFFF);
        idt_entries[32 + i].selector    = 0x08;
        idt_entries[32 + i].flags       = flags;
        idt_entries[32 + i].offset_mid  = (uint16_t)(((uint64_t)irq_stubs[i] >> 16) & 0xFFFF);
        idt_entries[32 + i].offset_high = (uint32_t)(((uint64_t)irq_stubs[i] >> 32) & 0xFFFFFFFF);
    }

    pic_remap();

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    pit_init(100);

    idt_descriptor.limit = (sizeof(idt_entries) - 1);
    idt_descriptor.base = (uint64_t)&idt_entries;

    idt_load_asm(&idt_descriptor);

    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    asm volatile("sti");
}
