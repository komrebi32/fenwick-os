#include "lib/libk/libk.h"

void krnl_main() {
    kclear_screen();
    kset_color(0x1F);
    kputs("========================================\n");
    kputs("  Welcome to FenwickOS!\n");
    kputs("========================================\n\n");
    kset_color(0x0F);
    kprintf("krnl_main loaded at %p\n", (void*)krnl_main);
    kputs("libk loaded successfully\n");
    while (1);
}
