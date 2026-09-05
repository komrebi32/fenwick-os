#ifndef RTC_H
#define RTC_H

#include <stddef.h>

struct rtc_time {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
    uint8_t weekday;
};

void rtc_init(void);
void rtc_get_time(struct rtc_time *t);
void rtc_print_time(void);

uint8_t rtc_bcd_to_bin(uint8_t val);
uint8_t rtc_bin_to_bcd(uint8_t val);

#endif
