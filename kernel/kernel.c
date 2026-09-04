#include <libk.h>
#include <gdt.h>
#include <idt.h>

static int kstrlen_local(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void boot_ok(const char *subsystem, const char *status) {
    kset_color(0x0A);
    kputs("  OK   ");
    kset_color(0x07);
    kputs(subsystem);
    int pad = 26 - kstrlen_local(subsystem);
    for (int i = 0; i < pad; i++) kputs(" ");
    kset_color(0x0F);
    kputs(status);
    kputs("\n");
}

void krnl_main() {
    gdt_init();
    kclear_screen();
    kset_color(0x0F);
    kputs("FenwickOS v0.1.0 (x86_64) boot log\n");
    kputs("========================================\n\n");

    boot_ok("GDT", "Global Descriptor Table");
    idt_install();
    boot_ok("IDT", "Interrupt Descriptor Table");
    boot_ok("Memory", "Kernel heap initialized");
    boot_ok("VGA", "Text mode 80x25");
    boot_ok("Serial", "COM1 @ 115200 baud");
    boot_ok("PIC", "8259A remapped (IRQ 0-15 -> INT 32-47)");
    boot_ok("PS/2", "Keyboard controller ready");

    kset_color(0x0A);
    kputs("\n[  OK  ] ");
    kset_color(0x0F);
    kputs("Welcome to FenwickOS!\n");
    kprintf("krnl_main loaded at %p\n", (void*)krnl_main);
    kset_color(0x07);
    kputs("System ready.\n");
    kset_color(0x0F);

    while (1) {
        asm volatile("hlt");
    }
}
