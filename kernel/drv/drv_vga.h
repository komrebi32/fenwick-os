#ifndef DRV_VGA_H
#define DRV_VGA_H

#include "drv.h"

int  vga_driver_init(device_t *dev);
int  vga_driver_probe(device_t *dev);
int  vga_driver_write(device_t *dev, const void *buf, uint64_t len);
int  vga_driver_read(device_t *dev, void *buf, uint64_t len);
int  vga_driver_ioctl(device_t *dev, uint32_t cmd, uint64_t arg);

extern drv_ops_t vga_ops;
extern driver_t  vga_driver;

#endif
