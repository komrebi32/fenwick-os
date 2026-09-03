#ifndef LIBK_KASSERT_H
#define LIBK_KASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

void kpanic(const char* msg);
void kassert(unsigned int cond, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
