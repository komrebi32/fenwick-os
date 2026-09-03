#include <libk.h>
#include <stdarg.h>

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kprintf_va(fmt, (const unsigned long long*)args);
    va_end(args);
}
