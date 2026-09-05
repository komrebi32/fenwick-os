#ifndef DRV_KBD_H
#define DRV_KBD_H

#include "drv.h"

int  kbd_driver_init(device_t *dev);
int  kbd_driver_probe(device_t *dev);
int  kbd_driver_read(device_t *dev, void *buf, uint64_t len);
int  kbd_driver_ioctl(device_t *dev, uint32_t cmd, uint64_t arg);

extern drv_ops_t kbd_ops;
extern driver_t  kbd_driver;

void kbd_handler_isr(void);

#endif
