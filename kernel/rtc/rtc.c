#include "rtc.h"
#include <libk.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

#define RTC_SECOND   0x00
#define RTC_MINUTE   0x02
#define RTC_HOUR     0x04
#define RTC_DAY      0x07
#define RTC_MONTH    0x08
#define RTC_YEAR     0x09
#define RTC_CENTURY  0x32
#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B
#define RTC_STATUS_C 0x0C

static inline void cmos_write(uint8_t reg, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(reg), "Nd"((uint16_t)CMOS_ADDRESS));
    asm volatile("outb %0, %1" : : "a"(val), "Nd"((uint16_t)CMOS_DATA));
}

static inline uint8_t cmos_read(uint8_t reg) {
    asm volatile("outb %0, %1" : : "a"(reg), "Nd"((uint16_t)CMOS_ADDRESS));
    uint8_t val;
    asm volatile("inb %1, %0" : "=a"(val) : "Nd"((uint16_t)CMOS_DATA));
    return val;
}

uint8_t rtc_bcd_to_bin(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

uint8_t rtc_bin_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t rtc_read_reg(uint8_t reg) {
    return cmos_read(reg);
}

void rtc_init(void) {
    uint8_t status_b = rtc_read_reg(RTC_STATUS_B);
    cmos_write(RTC_STATUS_B, status_b | 0x40);
    (void)rtc_read_reg(RTC_STATUS_C);
}

void rtc_get_time(struct rtc_time *t) {
    t->second  = rtc_read_reg(RTC_SECOND);
    t->minute  = rtc_read_reg(RTC_MINUTE);
    t->hour    = rtc_read_reg(RTC_HOUR);
    t->day     = rtc_read_reg(RTC_DAY);
    t->month   = rtc_read_reg(RTC_MONTH);
    t->year    = rtc_read_reg(RTC_YEAR);
    t->century = rtc_read_reg(RTC_CENTURY);

    uint8_t status_b = rtc_read_reg(RTC_STATUS_B);

    if (!(status_b & 0x04)) {
        t->second  = rtc_bcd_to_bin(t->second);
        t->minute  = rtc_bcd_to_bin(t->minute);
        t->hour    = rtc_bcd_to_bin(t->hour & 0x7F);
        t->day     = rtc_bcd_to_bin(t->day);
        t->month   = rtc_bcd_to_bin(t->month);
        t->year    = rtc_bcd_to_bin(t->year);
        t->century = rtc_bcd_to_bin(t->century);
    }

    if (!(status_b & 0x02) && (t->hour & 0x80)) {
        t->hour = ((t->hour & 0x7F) + 12) % 24;
    }

    t->weekday = 0;
}

void rtc_print_time(void) {
    struct rtc_time t;
    rtc_get_time(&t);

    const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    (void)days;

    kset_color(0x0F);
    kprintf("  Date: %d century, year %d, month %d, day %d\n",
            (int)t.century, (int)t.year, (int)t.month, (int)t.day);
    kprintf("  Time: %02d:%02d:%02d\n", (int)t.hour, (int)t.minute, (int)t.second);
}
