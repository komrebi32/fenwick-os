#include "drv_vga.h"
#include <libk.h>

static const uint8_t DEFAULT_COLOR = 0x0F;

static volatile uint8_t *vga_buffer = (volatile uint8_t *)0xB8000;
static const int VGA_WIDTH  = 80;
static const int VGA_HEIGHT = 25;

static int vga_cursor_x = 0;
static int vga_cursor_y = 0;
static uint8_t vga_color  = 0x0F;

static inline int vga_index(int x, int y) {
    return (y * VGA_WIDTH + x) * 2;
}

static void vga_putc_at(char c, uint8_t color, int x, int y) {
    int idx = vga_index(x, y);
    vga_buffer[idx] = (uint8_t)c;
    vga_buffer[idx + 1] = color;
}

static void vga_scroll(void) {
    if (vga_cursor_y < VGA_HEIGHT) return;
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int src = vga_index(x, y);
            int dst = vga_index(x, y - 1);
            vga_buffer[dst] = vga_buffer[src];
            vga_buffer[dst + 1] = vga_buffer[src + 1];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        int idx = vga_index(x, VGA_HEIGHT - 1);
        vga_buffer[idx] = ' ';
        vga_buffer[idx + 1] = vga_color;
    }
    vga_cursor_y = VGA_HEIGHT - 1;
}

static void vga_putc(char c) {
    if (c == '\n') {
        vga_cursor_x = 0;
        vga_cursor_y++;
        vga_scroll();
    } else if (c == '\r') {
        vga_cursor_x = 0;
    } else if (c == '\t') {
        int tab = 4 - (vga_cursor_x % 4);
        for (int i = 0; i < tab; i++) vga_putc(' ');
    } else if (c == '\b') {
        if (vga_cursor_x > 0) {
            vga_cursor_x--;
            vga_putc_at(' ', vga_color, vga_cursor_x, vga_cursor_y);
        }
    } else {
        vga_putc_at(c, vga_color, vga_cursor_x, vga_cursor_y);
        vga_cursor_x++;
        if (vga_cursor_x >= VGA_WIDTH) {
            vga_cursor_x = 0;
            vga_cursor_y++;
            vga_scroll();
        }
    }
}

int vga_driver_init(device_t *dev) {
    (void)dev;
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_putc_at(' ', DEFAULT_COLOR, x, y);
        }
    }
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    vga_color = DEFAULT_COLOR;
    return 0;
}

int vga_driver_probe(device_t *dev) {
    (void)dev;
    return 0;
}

int vga_driver_write(device_t *dev, const void *buf, uint64_t len) {
    (void)dev;
    if (!buf) return -1;
    const char *s = (const char *)buf;
    for (uint64_t i = 0; i < len; i++) {
        vga_putc(s[i]);
    }
    return (int)len;
}

int vga_driver_read(device_t *dev, void *buf, uint64_t len) {
    (void)dev; (void)buf; (void)len;
    return -1;
}

int vga_driver_ioctl(device_t *dev, uint32_t cmd, uint64_t arg) {
    (void)dev;
    switch (cmd) {
        case 0: vga_color = (uint8_t)arg; return 0;
        case 1: {
            vga_cursor_x = (int)arg;
            return 0;
        }
        case 2: {
            vga_cursor_y = (int)(arg >> 32);
            return 0;
        }
        case 3: {
            vga_cursor_x = 0;
            vga_cursor_y = 0;
            return 0;
        }
        default: return -1;
    }
}

drv_ops_t vga_ops = {
    .name     = "vga",
    .type     = DRV_TYPE_DISPLAY,
    .init     = vga_driver_init,
    .probe    = vga_driver_probe,
    .read     = vga_driver_read,
    .write    = vga_driver_write,
    .ioctl    = vga_driver_ioctl,
    .shutdown = 0,
};

driver_t vga_driver = {
    .name      = "vga",
    .type      = DRV_TYPE_DISPLAY,
    .ops       = &vga_ops,
};
