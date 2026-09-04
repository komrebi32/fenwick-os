#ifndef GDT_H
#define GDT_H

#include <stddef.h>

#define GDT_ENTRIES 8

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint64_t reserved4;
    uint64_t reserved5;
    uint32_t reserved6;
    uint32_t reserved7;
    uint32_t reserved8;
    uint32_t reserved9;
} __attribute__((packed));

void gdt_init(void);
void gdt_load(uint64_t gdt_ptr);
void gdt_flush_tss(void);

#endif
