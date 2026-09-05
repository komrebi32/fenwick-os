#include "drv_serial.h"

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void serial_putc(char c) {
    while (!(inb(COM1 + 5) & 0x20));
    outb(COM1, (uint8_t)c);
}

int serial_driver_init(device_t *dev) {
    (void)dev;
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    return 0;
}

int serial_driver_write(device_t *dev, const void *buf, uint64_t len) {
    (void)dev;
    if (!buf) return -1;
    const char *s = (const char *)buf;
    for (uint64_t i = 0; i < len; i++) {
        serial_putc(s[i]);
    }
    return (int)len;
}

int serial_driver_read(device_t *dev, void *buf, uint64_t len) {
    (void)dev; (void)buf; (void)len;
    return -1;
}

drv_ops_t serial_ops = {
    .name     = "serial",
    .type     = DRV_TYPE_SERIAL,
    .init     = serial_driver_init,
    .probe    = 0,
    .read     = serial_driver_read,
    .write    = serial_driver_write,
    .ioctl    = 0,
    .shutdown = 0,
};

driver_t serial_driver = {
    .name = "serial",
    .type = DRV_TYPE_SERIAL,
    .ops  = &serial_ops,
};
