#include "drv.h"
#include <libk.h>

static driver_t drivers[MAX_DRIVERS];
static device_t devices[MAX_DEVICES];
static int driver_count = 0;
static int device_count = 0;
static int drv_initialized = 0;

void drv_init(void) {
    if (drv_initialized) return;
    for (int i = 0; i < MAX_DRIVERS; i++) {
        drivers[i].registered = 0;
        drivers[i].probed = 0;
        drivers[i].attached = 0;
    }
    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].active = 0;
        devices[i].id = 0;
    }
    driver_count = 0;
    device_count = 0;
    drv_initialized = 1;
}

int drv_register(driver_t *drv) {
    if (!drv || !drv->ops) return -1;
    if (driver_count >= MAX_DRIVERS) return -1;

    int i;
    for (i = 0; i < MAX_DRIVERS; i++) {
        if (!drivers[i].registered) break;
    }
    if (i >= MAX_DRIVERS) return -1;

    drivers[i] = *drv;
    drivers[i].registered = 1;
    driver_count++;
    return 0;
}

int drv_unregister(driver_t *drv) {
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (&drivers[i] == drv) {
            drivers[i].registered = 0;
            driver_count--;
            return 0;
        }
    }
    return -1;
}

int dev_register(device_t *dev) {
    if (!dev) return -1;
    if (device_count >= MAX_DEVICES) return -1;

    int i;
    for (i = 0; i < MAX_DEVICES; i++) {
        if (!devices[i].active) break;
    }
    if (i >= MAX_DEVICES) return -1;

    if (dev->id == 0) {
        dev->id = i + 1;
    }
    devices[i] = *dev;
    devices[i].active = 1;
    device_count++;
    return 0;
}

device_t *dev_get(uint32_t id) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active && devices[i].id == id) {
            return &devices[i];
        }
    }
    return 0;
}

device_t *dev_get_by_name(const char *name) {
    if (!name) return 0;
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active) {
            int j = 0;
            while (name[j] && devices[i].name[j] == name[j] && j < DRV_NAME_MAX - 1) j++;
            if (name[j] == 0 && devices[i].name[j] == 0) {
                return &devices[i];
            }
        }
    }
    return 0;
}

device_t *dev_get_by_type(drv_type_t type) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active && devices[i].type == type) {
            return &devices[i];
        }
    }
    return 0;
}

int drv_attach(device_t *dev, driver_t *drv) {
    if (!dev || !drv) return -1;
    dev->driver = drv;
    return 0;
}

int drv_probe_all(void) {
    int count = 0;
    for (int d = 0; d < MAX_DRIVERS; d++) {
        if (!drivers[d].registered) continue;
        for (int i = 0; i < MAX_DEVICES; i++) {
            if (!devices[i].active) continue;
            if (devices[i].type != drivers[d].type) continue;
            if (devices[i].driver) continue;

            if (drivers[d].ops && drivers[d].ops->probe) {
                if (drivers[d].ops->probe(&devices[i]) == 0) {
                    drv_attach(&devices[i], &drivers[d]);
                    drivers[d].probed = 1;
                    devices[i].driver = &drivers[d];
                    count++;
                }
            }
        }
    }
    return count;
}

int drv_call_init(device_t *dev) {
    if (!dev || !dev->driver || !dev->driver->ops) return -1;
    if (!dev->driver->ops->init) return -1;
    return dev->driver->ops->init(dev);
}

int drv_call_read(device_t *dev, void *buf, uint64_t len) {
    if (!dev || !dev->driver || !dev->driver->ops) return -1;
    if (!dev->driver->ops->read) return -1;
    return dev->driver->ops->read(dev, buf, len);
}

int drv_call_write(device_t *dev, const void *buf, uint64_t len) {
    if (!dev || !dev->driver || !dev->driver->ops) return -1;
    if (!dev->driver->ops->write) return -1;
    return dev->driver->ops->write(dev, buf, len);
}

int drv_call_ioctl(device_t *dev, uint32_t cmd, uint64_t arg) {
    if (!dev || !dev->driver || !dev->driver->ops) return -1;
    if (!dev->driver->ops->ioctl) return -1;
    return dev->driver->ops->ioctl(dev, cmd, arg);
}

void drv_call_shutdown(device_t *dev) {
    if (!dev || !dev->driver || !dev->driver->ops) return;
    if (dev->driver->ops->shutdown) {
        dev->driver->ops->shutdown(dev);
    }
}

void drv_list(void) {
    kset_color(0x0F);
    kputs("Registered Drivers:\n");
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (drivers[i].registered) {
            kprintf("  [%d] %s (type=%d)\n", i, drivers[i].name, drivers[i].type);
        }
    }

    kset_color(0x0F);
    kputs("Active Devices:\n");
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active) {
            const char *drv_name = "none";
            if (devices[i].driver) drv_name = devices[i].driver->name;
            kprintf("  [%d] %s (type=%d, drv=%s)\n",
                    devices[i].id, devices[i].name, devices[i].type, drv_name);
        }
    }
}
