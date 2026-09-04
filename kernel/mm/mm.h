#ifndef MM_H
#define MM_H

#include <stddef.h>

#define PAGE_SIZE       4096
#define PAGE_SHIFT      12
#define PAGES_PER_TABLE 512
#define PAGES_PER_DIR  512

#define PAGE_PRESENT    0x01
#define PAGE_WRITABLE   0x02
#define PAGE_USER       0x04
#define PAGE_ACCESSED   0x20
#define PAGE_DIRTY      0x40
#define PAGE_HUGE       0x80

#define USER_STACK_TOP  0x0000700000000000ULL
#define USER_BASE       0x0000000000400000ULL
#define KERNEL_BASE     0xFFFFFFFF80000000ULL

#define PHYS_MEM_SIZE   0x10000000ULL
#define MAX_PAGES       16384

struct page {
    uint64_t phys_addr;
    uint8_t  present;
    uint8_t  writable;
    uint8_t  user;
    uint8_t  reserved;
};

struct page_table {
    uint64_t entries[PAGES_PER_TABLE];
};

struct page_directory {
    uint64_t entries[PAGES_PER_DIR];
};

struct phys_mem_region {
    uint64_t base;
    uint64_t size;
    uint8_t  used;
    uint8_t  type;
    uint32_t reserved;
};

#define MAX_REGIONS 64
#define HEAP_START  0x100000
#define HEAP_SIZE   0x800000

void mm_init(void);
void paging_init(void);
void mm_start(void);
int  mm_is_ready(void);
void mm_print_stats(void);

uint64_t mm_alloc_page(void);
void mm_free_page(uint64_t phys);

void *mm_map_page(uint64_t phys, uint64_t virt, uint64_t flags);
void *mm_unmap_page(uint64_t virt);

void *kmalloc(uint64_t size);
void  kfree(void *ptr);

void paging_switch_directory(struct page_directory *dir);
void paging_invalidate(uint64_t virt);

void mm_dump_page_table(uint64_t virt);

#endif
