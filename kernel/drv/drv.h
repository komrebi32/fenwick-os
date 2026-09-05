#ifndef DRV_H
#define DRV_H

#include <stddef.h>

#define DRV_NAME_MAX 32
#define MAX_DRIVERS  16
#define MAX_DEVICES  32

typedef enum {
    DRV_TYPE_UNKNOWN = 0,
    DRV_TYPE_KEYBOARD,
    DRV_TYPE_RTC,
    DRV_TYPE_DISPLAY,
    DRV_TYPE_SERIAL,
    DRV_TYPE_TIMER,
    DRV_TYPE_PIC,
    DRV_TYPE_RAM,
} drv_type_t;

struct device;
struct driver;

typedef int  (*drv_init_fn)  (struct device *dev);
typedef int  (*drv_probe_fn) (struct device *dev);
typedef int  (*drv_read_fn)  (struct device *dev, void *buf, uint64_t len);
typedef int  (*drv_write_fn) (struct device *dev, const void *buf, uint64_t len);
typedef int  (*drv_ioctl_fn) (struct device *dev, uint32_t cmd, uint64_t arg);
typedef void (*drv_shutdown_fn)(struct device *dev);

typedef struct drv_ops {
    const char *name;
    drv_type_t  type;
    drv_init_fn    init;
    drv_probe_fn   probe;
    drv_read_fn    read;
    drv_write_fn   write;
    drv_ioctl_fn   ioctl;
    drv_shutdown_fn shutdown;
} drv_ops_t;

typedef struct driver {
    char         name[DRV_NAME_MAX];
    drv_type_t   type;
    drv_ops_t   *ops;
    uint8_t      registered;
    uint8_t      probed;
    uint8_t      attached;
    uint8_t      reserved;
} driver_t;

typedef struct device {
    char         name[DRV_NAME_MAX];
    drv_type_t   type;
    driver_t    *driver;
    void        *state;
    uint32_t     id;
    uint8_t      active;
    uint8_t      reserved[3];
} device_t;

void drv_init(void);
int  drv_register(driver_t *drv);
int  drv_unregister(driver_t *drv);
int  drv_probe_all(void);
int  drv_attach(device_t *dev, driver_t *drv);

int  dev_register(device_t *dev);
device_t *dev_get(uint32_t id);
device_t *dev_get_by_name(const char *name);
device_t *dev_get_by_type(drv_type_t type);

int  drv_call_init(device_t *dev);
int  drv_call_read(device_t *dev, void *buf, uint64_t len);
int  drv_call_write(device_t *dev, const void *buf, uint64_t len);
int  drv_call_ioctl(device_t *dev, uint32_t cmd, uint64_t arg);
void drv_call_shutdown(device_t *dev);

void drv_list(void);

#endif
