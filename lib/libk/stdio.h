#ifndef LIBK_STDIO_H
#define LIBK_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

void kputc(char c);
void kputs(const char* s);
void kprintf(const char* fmt, ...);
void kprintf_va(const char* fmt, const unsigned long long* args);
void kset_color(unsigned char color);
void kclear_screen(void);
void print_hex64(unsigned long long val);

#ifdef __cplusplus
}
#endif

#endif
