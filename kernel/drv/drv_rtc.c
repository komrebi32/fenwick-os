#include "drv_rtc.h"
#include <libk.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
} rtc_time_t;

static inline void cmos_write(uint8_t reg, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(reg), "Nd"((uint16_t)CMOS_ADDR));
    asm volatile("outb %0, %1" : : "a"(val), "Nd"((uint16_t)CMOS_DATA));
}

static inline uint8_t cmos_read(uint8_t reg) {
    asm volatile("outb %0, %1" : : "a"(reg), "Nd"((uint16_t)CMOS_ADDR));
    uint8_t val;
    asm volatile("inb %1, %0" : "=a"(val) : "Nd"((uint16_t)CMOS_DATA));
    return val;
}

static uint8_t bcd_to_bin(uint8_t v) {
    return (v >> 4) * 10 + (v & 0x0F);
}

static void rtc_get_raw(rtc_time_t *t) {
    t->second  = cmos_read(0x00);
    t->minute  = cmos_read(0x02);
    t->hour    = cmos_read(0x04);
    t->day     = cmos_read(0x07);
    t->month   = cmos_read(0x08);
    t->year    = cmos_read(0x09);
    t->century = cmos_read(0x32);
    (void)cmos_read(0x0C);

    uint8_t sb = cmos_read(0x0B);
    if (!(sb & 0x04)) {
        t->second  = bcd_to_bin(t->second);
        t->minute  = bcd_to_bin(t->minute);
        t->hour    = bcd_to_bin(t->hour & 0x7F);
        t->day     = bcd_to_bin(t->day);
        t->month   = bcd_to_bin(t->month);
        t->year    = bcd_to_bin(t->year);
        t->century = bcd_to_bin(t->century);
    }
    if (!(sb & 0x02) && (t->hour & 0x80)) {
        t->hour = ((t->hour & 0x7F) + 12) % 24;
    }
}

int rtc_driver_init(device_t *dev) {
    (void)dev;
    cmos_write(0x0B, cmos_read(0x0B) | 0x40);
    (void)cmos_read(0x0C);
    return 0;
}

int rtc_driver_probe(device_t *dev) {
    (void)dev;
    return 0;
}

int rtc_driver_read(device_t *dev, void *buf, uint64_t len) {
    (void)dev;
    if (!buf || len < sizeof(rtc_time_t)) return -1;
    rtc_time_t *t = (rtc_time_t *)buf;
    rtc_get_raw(t);
    return sizeof(rtc_time_t);
}

int rtc_driver_ioctl(device_t *dev, uint32_t cmd, uint64_t arg) {
    (void)dev; (void)arg;
    switch (cmd) {
        case 0: return 100;
        default: return -1;
    }
}

drv_ops_t rtc_ops = {
    .name     = "cmos-rtc",
    .type     = DRV_TYPE_RTC,
    .init     = rtc_driver_init,
    .probe    = rtc_driver_probe,
    .read     = rtc_driver_read,
    .write    = 0,
    .ioctl    = rtc_driver_ioctl,
    .shutdown = 0,
};

driver_t rtc_driver = {
    .name = "cmos-rtc",
    .type = DRV_TYPE_RTC,
    .ops  = &rtc_ops,
};
