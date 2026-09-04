#ifndef IDT_H
#define IDT_H

#include <stddef.h>

#define IDT_ENTRIES 256

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_install(void);
void isr_handler(uint64_t int_no, uint64_t err_code);
void irq_handler(uint64_t int_no);

#endif
