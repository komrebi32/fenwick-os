#ifndef PANIC_H
#define PANIC_H

#include <stddef.h>

void panic_handler(const char *message, uint64_t int_no, uint64_t err_code);
void panic(const char *message);

#endif
