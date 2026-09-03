#ifndef LIBK_MEMORY_H
#define LIBK_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

void kheap_init(unsigned long start, unsigned long size);
void kpage_init(void);
void* kmalloc(unsigned long size);
void* kmalloc_aligned(unsigned long size, unsigned long align);
void kfree(void* ptr);
void* krealloc(void* ptr, unsigned long old_size, unsigned long new_size);

#ifdef __cplusplus
}
#endif

#endif
