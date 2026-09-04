#include "panic.h"
#include <libk.h>

#define BSOD_BG        0x1F
#define BSOD_TEXT      0x1F
#define BSOD_HIGHLIGHT 0x1E

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void fill_screen(uint8_t color) {
    volatile uint8_t *vga = (volatile uint8_t *)0xB8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vga[i] = ' ';
        vga[i + 1] = color;
    }
}

void panic_handler(const char *message, uint64_t int_no, uint64_t err_code) {
    asm volatile("cli");

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    kset_color(BSOD_BG);
    kclear_screen();
    fill_screen(BSOD_BG);

    kset_color(BSOD_TEXT);
    kputs("\n");
    kputs(":(  FenwickOS has encountered a problem and needs to restart.\n");
    kputs("\n");
    kputs("We're just collecting some error info, and then we'll restart for you.\n");
    kputs("\n");

    kset_color(BSOD_TEXT);
    kputs("  ");
    kset_color(BSOD_HIGHLIGHT);
    kputs("Code:  ");
    kset_color(BSOD_TEXT);
    kputs("INT#");
    print_hex64(int_no);
    kputs("  ERR=");
    print_hex64(err_code);
    kputs("\n");

    kputs("\n");
    kset_color(BSOD_TEXT);
    kputs("  What happened: ");
    kputs(message);
    kputs("\n\n");

    kset_color(BSOD_TEXT);
    kputs("  If you'd like to know more, search online for the error code above.\n\n");

    kset_color(BSOD_HIGHLIGHT);
    kputs("  Technical information:\n");
    kset_color(BSOD_TEXT);
    kputs("  *** STOP: 0x");
    print_hex64((uint64_t)message);
    kputs(" (");
    print_hex64(int_no);
    kputs("-");
    print_hex64(err_code);
    kputs(")\n\n");

    kset_color(BSOD_HIGHLIGHT);
    kputs("  Press any key to continue_");
    kset_color(BSOD_TEXT);

    asm volatile("hlt");
    while (1) {
        asm volatile("hlt");
    }
}

void panic(const char *message) {
    panic_handler(message, 0, 0);
}
