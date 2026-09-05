#include "drv.h"
#include "drv/drv_kbd.h"
#include "drv/drv_rtc.h"
#include "drv/drv_vga.h"
#include "drv/drv_serial.h"
#include "vfs/vfs.h"
#include <libk.h>

extern device_t *dev_get_by_type_internal(drv_type_t type);

static const char *type_name(drv_type_t t) {
    switch (t) {
        case DRV_TYPE_KEYBOARD: return "keyboard";
        case DRV_TYPE_RTC:      return "rtc";
        case DRV_TYPE_DISPLAY:  return "display";
        case DRV_TYPE_SERIAL:   return "serial";
        case DRV_TYPE_TIMER:    return "timer";
        case DRV_TYPE_PIC:      return "pic";
        case DRV_TYPE_RAM:      return "ram";
        default:                return "unknown";
    }
}

static void copy_name(char *dst, const char *src) {
    int j = 0;
    while (src[j] && j < DRV_NAME_MAX - 1) { dst[j] = src[j]; j++; }
    dst[j] = 0;
}

void subsystem_init(void) {
    kset_color(0x0F);
    kputs("[subsystem] Initializing driver framework...\n");

    drv_init();
    vfs_init();

    drv_register(&vga_driver);
    drv_register(&serial_driver);
    drv_register(&rtc_driver);
    drv_register(&kbd_driver);

    device_t vga_dev;
    vga_dev.type = DRV_TYPE_DISPLAY;
    copy_name(vga_dev.name, "vga0");
    vga_dev.driver = 0;
    vga_dev.state = 0;
    vga_dev.id = 1;
    vga_dev.active = 0;
    vga_dev.reserved[0] = 0;
    vga_dev.reserved[1] = 0;
    vga_dev.reserved[2] = 0;
    dev_register(&vga_dev);

    device_t serial_dev;
    serial_dev.type = DRV_TYPE_SERIAL;
    copy_name(serial_dev.name, "com1");
    serial_dev.driver = 0;
    serial_dev.state = 0;
    serial_dev.id = 2;
    serial_dev.active = 0;
    serial_dev.reserved[0] = 0;
    serial_dev.reserved[1] = 0;
    serial_dev.reserved[2] = 0;
    dev_register(&serial_dev);

    device_t rtc_dev;
    rtc_dev.type = DRV_TYPE_RTC;
    copy_name(rtc_dev.name, "rtc0");
    rtc_dev.driver = 0;
    rtc_dev.state = 0;
    rtc_dev.id = 3;
    rtc_dev.active = 0;
    rtc_dev.reserved[0] = 0;
    rtc_dev.reserved[1] = 0;
    rtc_dev.reserved[2] = 0;
    dev_register(&rtc_dev);

    device_t kbd_dev;
    kbd_dev.type = DRV_TYPE_KEYBOARD;
    copy_name(kbd_dev.name, "kbd0");
    kbd_dev.driver = 0;
    kbd_dev.state = 0;
    kbd_dev.id = 4;
    kbd_dev.active = 0;
    kbd_dev.reserved[0] = 0;
    kbd_dev.reserved[1] = 0;
    kbd_dev.reserved[2] = 0;
    dev_register(&kbd_dev);

    int probed = drv_probe_all();
    kprintf("[subsystem] Probed %d devices\n", probed);

    for (int i = 0; i < MAX_DEVICES; i++) {
        device_t *dev = dev_get(i + 1);
        if (dev && dev->active && dev->driver) {
            drv_call_init(dev);
        }
    }

    device_t *vga = dev_get_by_name("vga0");
    device_t *ser = dev_get_by_name("com1");
    device_t *rtc = dev_get_by_name("rtc0");
    device_t *kbd = dev_get_by_name("kbd0");

    vfs_register(VFS_NODE_DISPLAY, "vga0",  DRV_TYPE_DISPLAY, vga);
    vfs_register(VFS_NODE_SERIAL,  "com1",  DRV_TYPE_SERIAL,  ser);
    vfs_register(VFS_NODE_RTC,     "rtc0",  DRV_TYPE_RTC,     rtc);
    vfs_register(VFS_NODE_KBD,     "kbd0",  DRV_TYPE_KEYBOARD, kbd);
    vfs_register(VFS_NODE_STDOUT,  "stdout", DRV_TYPE_DISPLAY, vga);
    vfs_register(VFS_NODE_KBDIN,   "kbdi",  DRV_TYPE_KEYBOARD, kbd);

    kprintf("[subsystem] VFS registered\n");
    kputs("[subsystem] Driver framework ready\n");
}
