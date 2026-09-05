#include <libk.h>
#include <gdt.h>
#include <idt.h>
#include <panic.h>
#include "mm/mm.h"
#include "drv/drv.h"
#include "drv/drv_kbd.h"
#include "vfs/vfs.h"
#include "syscall/syscall.h"

void subsystem_init(void);

static int kstrlen_local(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void boot_ok(const char *subsystem, const char *status) {
    vfs_write_str(VFS_NODE_STDOUT, "  OK   ");
    vfs_write_str(VFS_NODE_STDOUT, subsystem);
    int pad = 26 - kstrlen_local(subsystem);
    for (int i = 0; i < pad; i++) vfs_write_str(VFS_NODE_STDOUT, " ");
    vfs_write_str(VFS_NODE_STDOUT, status);
    vfs_write_str(VFS_NODE_STDOUT, "\n");
}

static void show_shell_info(void) {
    vfs_write_str(VFS_NODE_STDOUT, "\n");
    vfs_write_str(VFS_NODE_STDOUT, "========================================\n");
    vfs_write_str(VFS_NODE_STDOUT, "  FenwickOS - Preparing Ring 3 jump\n");
    vfs_write_str(VFS_NODE_STDOUT, "========================================\n\n");
    vfs_write_str(VFS_NODE_STDOUT, "[OK] Kernel ready for userland\n");
    vfs_write_str(VFS_NODE_STDOUT, "[OK] TSS configured\n");
    vfs_write_str(VFS_NODE_STDOUT, "[OK] Syscall MSRs configured\n");
    vfs_write_str(VFS_NODE_STDOUT, "[OK] GDT user segments (0x23, 0x1B)\n");
    vfs_write_str(VFS_NODE_STDOUT, "[OK] Loading sh.elf at 0x400000\n");
    vfs_write_str(VFS_NODE_STDOUT, "\n");
}

extern uint8_t _binary_sh_elf_start[];
extern uint8_t _binary_sh_elf_end[];

void krnl_main() {
    gdt_init();

    kset_color(0x0F);
    kprintf("FenwickOS v0.1.0 (x86_64) boot log\n");
    kprintf("========================================\n\n");

    mm_start();
    boot_ok("Memory", "4KB paging + free-list heap");

    idt_install();
    boot_ok("IDT", "Interrupt Descriptor Table");

    subsystem_init();
    boot_ok("Subsystem", "Driver framework + VFS");
    boot_ok("Display", "VGA text mode 80x25");
    boot_ok("Serial", "COM1 @ 115200 baud");
    boot_ok("Keyboard", "PS/2 controller");
    boot_ok("RTC", "CMOS Real-Time Clock");
    boot_ok("Timer", "PIT 100Hz (10ms ticks)");
    boot_ok("PIC", "8259A remapped");

    kset_color(0x0A);
    kputs("\n[  OK  ] Welcome to FenwickOS!\n");
    kset_color(0x0F);
    kputs("krnl_main loaded at ");
    kprintf("0x%p\n", (void*)krnl_main);
    kset_color(0x07);
    kputs("Loading userland (sh.elf) and jumping to Ring 3...\n");
    kset_color(0x0F);

    tss_init();
    syscall_init();

    show_shell_info();

    uint8_t *sh_data = (uint8_t *)0x400000;
    for (uint8_t *p = sh_data; p < sh_data + 0x10000; p++) *p = 0;

    extern uint8_t _binary_userland_system_sh_sh_bin_start[];
    extern uint8_t _binary_userland_system_sh_sh_bin_end[];
    uint8_t *src = _binary_userland_system_sh_sh_bin_start;
    uint8_t *dst = sh_data;
    uint64_t size = (uint64_t)(_binary_userland_system_sh_sh_bin_end - _binary_userland_system_sh_sh_bin_start);
    if (size > 0x10000) size = 0x10000;
    for (uint64_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    uint64_t user_stack = 0x500000;
    uint64_t entry = 0x400000;

    kprintf("Jumping to Ring 3 at 0x%llx...\n", (unsigned long long)entry);

    extern void userland_enter(uint64_t entry, uint64_t stack);
    userland_enter(entry, user_stack);

    while (1) {
        asm volatile("hlt");
    }
}
