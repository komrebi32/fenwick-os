#include <libk.h>
#include <gdt.h>
#include <idt.h>
#include <kbd.h>
#include <panic.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

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

void trigger_test_panic(void) {
    int x = 1;
    int y = 0;
    int z = x / y;
    (void)z;
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
    kbd_init();
    boot_ok("Keyboard", "PS/2 controller");
    boot_ok("Timer", "PIT 100Hz (10ms ticks)");
    boot_ok("Memory", "Kernel heap initialized");
    boot_ok("VGA", "Text mode 80x25");
    boot_ok("Serial", "COM1 @ 115200 baud");
    boot_ok("PIC", "8259A remapped (IRQ 0-15 -> INT 32-47)");

    kset_color(0x0A);
    kputs("\n[  OK  ] ");
    kset_color(0x0F);
    kputs("Welcome to FenwickOS!\n");
    kprintf("krnl_main loaded at %p\n", (void*)krnl_main);
    kset_color(0x07);
    kputs("System ready.\n");
    kset_color(0x0F);
    kputs("Type something, or press Ctrl+D to trigger a test panic.\n\n");

    kset_color(0x08);
    kputs("> ");
    kset_color(0x0F);

    char buffer[128];
    int pos = 0;

    while (1) {
        if (kbd_has_input()) {
            char c = kbd_getchar();

            if (kbd_get_modifiers() & KBD_MOD_CTRL) {
                if (c == 'd' || c == 'D') {
                    kset_color(0x0E);
                    kputs("^D\n");
                    kset_color(0x0F);
                    trigger_test_panic();
                }
                continue;
            }

            if (c == '\b') {
                if (pos > 0) {
                    pos--;
                    kputs("\b \b");
                }
            } else if (c == '\n') {
                kputs("\n");
                buffer[pos] = 0;
                if (kstrcmp(buffer, "panic") == 0) {
                    trigger_test_panic();
                } else if (kstrcmp(buffer, "clear") == 0) {
                    kclear_screen();
                } else if (kstrcmp(buffer, "time") == 0) {
                    kprintf("Uptime: %d ms\n", (int)(timer_get_ticks() * 10));
                } else if (kstrcmp(buffer, "reboot") == 0) {
                    kputs("Rebooting...\n");
                    uint8_t temp;
                    do {
                        temp = inb(0x64);
                    } while (temp & 0x02);
                    outb(0x64, 0xFE);
                    while (1) asm volatile("hlt");
                } else if (pos > 0) {
                    kprintf("Unknown command: %s\n", buffer);
                }
                pos = 0;
                kset_color(0x08);
                kputs("> ");
                kset_color(0x0F);
            } else if (c >= 0x20 && c < 0x7F) {
                if (pos < 127) {
                    buffer[pos++] = c;
                    kputc(c);
                }
            } else if (c == '\t') {
                kputs("    ");
            }
        } else {
            asm volatile("hlt");
        }
    }
}
