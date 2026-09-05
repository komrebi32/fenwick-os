#ifndef DRV_SERIAL_H
#define DRV_SERIAL_H

#include "drv.h"

int  serial_driver_init(device_t *dev);
int  serial_driver_write(device_t *dev, const void *buf, uint64_t len);
int  serial_driver_read(device_t *dev, void *buf, uint64_t len);

extern drv_ops_t serial_ops;
extern driver_t  serial_driver;

#endif
