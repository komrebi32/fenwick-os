#ifndef DRV_RTC_H
#define DRV_RTC_H

#include "drv.h"

int  rtc_driver_init(device_t *dev);
int  rtc_driver_probe(device_t *dev);
int  rtc_driver_read(device_t *dev, void *buf, uint64_t len);
int  rtc_driver_ioctl(device_t *dev, uint32_t cmd, uint64_t arg);

extern drv_ops_t rtc_ops;
extern driver_t  rtc_driver;

#endif
