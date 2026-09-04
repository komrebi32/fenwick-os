#include "gdt.h"

static uint64_t gdt_entries[GDT_ENTRIES] __attribute__((aligned(8)));
static struct gdt_ptr gdt_descriptor;

void gdt_init(void) {
    gdt_entries[0] = 0x0000000000000000ULL;
    gdt_entries[1] = 0x00AF9B000000FFFFULL;
    gdt_entries[2] = 0x00AF93000000FFFFULL;
    gdt_entries[3] = 0x0000000000000000ULL;
    gdt_entries[4] = 0x0000000000000000ULL;
    gdt_entries[5] = 0x0000000000000000ULL;
    gdt_entries[6] = 0x0000000000000000ULL;
    gdt_entries[7] = 0x0000000000000000ULL;

    gdt_descriptor.limit = (sizeof(gdt_entries) - 1);
    gdt_descriptor.base = (uint64_t)&gdt_entries;

    gdt_load((uint64_t)&gdt_descriptor);
}
