#ifndef LIBK_H
#define LIBK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>

void* kmemcpy(void* dest, const void* src, unsigned long n);
void* kmemset(void* dest, int val, unsigned long n);
int kmemcmp(const void* s1, const void* s2, unsigned long n);
unsigned long kstrlen(const char* s);
char* kstrcpy(char* dest, const char* src);
char* kstrncpy(char* dest, const char* src, unsigned long n);
int kstrcmp(const char* s1, const char* s2);
int kstrncmp(const char* s1, const char* s2, unsigned long n);
char* kstrchr(const char* s, int c);
char* kstrrchr(const char* s, int c);
char* kstrcat(char* dest, const char* src);
char* kstrncat(char* dest, const char* src, unsigned long n);

void kputc(char c);
void kputs(const char* s);
void kprintf(const char* fmt, ...);
void kprintf_va(const char* fmt, const unsigned long long* args);
void kset_color(unsigned char color);
void kclear_screen(void);

void kheap_init(unsigned long start, unsigned long size);
void kpage_init(void);
void* kmalloc(unsigned long size);
void* kmalloc_aligned(unsigned long size, unsigned long align);
void kfree(void* ptr);
void* krealloc(void* ptr, unsigned long old_size, unsigned long new_size);

void kpanic(const char* msg);
void kassert(unsigned int cond, const char* msg);

#ifdef __cplusplus
}
#endif

#endif