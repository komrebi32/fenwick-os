#include "syscall.h"
#include <libk.h>
#include "drv/drv.h"
#include "vfs/vfs.h"
#include "idt/idt.h"

static syscall_fn_t syscalls[SYSCALL_MAX];
static int syscall_count = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

long sys_write(long fd, long buf, unsigned long len) {
    if (fd == 1 || fd == 2) {
        return vfs_write(VFS_NODE_STDOUT, (const void *)buf, len);
    }
    return -1;
}

long sys_read(long fd, long buf, unsigned long len) {
    if (fd == 0) {
        return vfs_read(VFS_NODE_KBD, (void *)buf, len);
    }
    return -1;
}

long sys_open(long path, long flags) {
    (void)path; (void)flags;
    return -1;
}

long sys_close(long fd) {
    (void)fd;
    return 0;
}

long sys_getchar(long unused) {
    (void)unused;
    return 0;
}

long sys_putchar(long c) {
    char tmp = (char)c;
    vfs_write(VFS_NODE_STDOUT, &tmp, 1);
    return 1;
}

long sys_exit(long code) {
    (void)code;
    kset_color(0x0E);
    kprintf("\n[exit] shell returned with code %d\n", code);
    while (1) asm volatile("hlt");
    return 0;
}

long sys_getpid(long unused) {
    (void)unused;
    return 1;
}

long sys_time(long unused) {
    (void)unused;
    return (long)(timer_get_ticks() * 10);
}

int syscall_register(int num, syscall_fn_t fn) {
    if (num < 0 || num >= SYSCALL_MAX || !fn) return -1;
    syscalls[num] = fn;
    if (num >= syscall_count) syscall_count = num + 1;
    return 0;
}

long syscall_handle(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
    if (num < 0 || num >= SYSCALL_MAX) return -1;
    if (!syscalls[num]) return -1;
    return syscalls[num](a1, a2, a3, a4, a5, a6);
}

void syscall_dispatcher(void) {
    (void)0;
}

void syscall_init(void) {
    syscall_register(SYSCALL_WRITE,   (syscall_fn_t)sys_write);
    syscall_register(SYSCALL_READ,    (syscall_fn_t)sys_read);
    syscall_register(SYSCALL_OPEN,    (syscall_fn_t)sys_open);
    syscall_register(SYSCALL_CLOSE,   (syscall_fn_t)sys_close);
    syscall_register(SYSCALL_GETCHAR, (syscall_fn_t)sys_getchar);
    syscall_register(SYSCALL_PUTCHAR, (syscall_fn_t)sys_putchar);
    syscall_register(SYSCALL_EXIT,    (syscall_fn_t)sys_exit);
    syscall_register(SYSCALL_GETPID,  (syscall_fn_t)sys_getpid);
    syscall_register(SYSCALL_TIME,    (syscall_fn_t)sys_time);

    uint64_t star = ((uint64_t)0x1B << 32) | (uint64_t)0x08;
    asm volatile("wrmsr" : : "a"((uint32_t)star), "d"((uint32_t)(star >> 32)), "c"(0xC0000081) : "memory");

    uint64_t lstar = (uint64_t)syscall_entry;
    asm volatile("wrmsr" : : "a"((uint32_t)lstar), "d"((uint32_t)(lstar >> 32)), "c"(0xC0000082) : "memory");

    uint64_t fmask = (uint64_t)0x200;
    asm volatile("wrmsr" : : "a"((uint32_t)fmask), "d"((uint32_t)(fmask >> 32)), "c"(0xC0000084) : "memory");
}

struct tss_entry_struct {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
    uint16_t reserved5;
    uint16_t iopb;
} __attribute__((packed));

static struct tss_entry_struct tss;

void tss_init(void) {
    for (int i = 0; i < sizeof(tss); i++) {
        ((volatile uint8_t*)&tss)[i] = 0;
    }
    tss.rsp0 = 0x70000;
    tss.iopb = sizeof(tss);

    uint16_t tss_sel = 0x2B;
    asm volatile("ltr %0" : : "r"(tss_sel) : "memory");
}

void tss_set_rsp0(uint64_t rsp0) {
    tss.rsp0 = rsp0;
}
