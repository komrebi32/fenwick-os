#ifndef LIBK_STRING_H
#define LIBK_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"

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

#define memset(s, c, n) kmemset((s), (c), (n))
#define memcpy(d, s, n) kmemcpy((d), (s), (n))
#define strcmp(s1, s2) kstrcmp((s1), (s2))
#define strlen(s) kstrlen((s))

#ifdef __cplusplus
}
#endif

#endif
