#include "mm.h"
#include <libk.h>

#define PAGES_TOTAL MAX_PAGES

#define MAX_PHYS_PAGES  262144
#define RAM_SIZE_GB     1

static struct phys_mem_region phys_regions[MAX_REGIONS];
static int phys_region_count = 0;

static struct page pages[MAX_PAGES];
static uint8_t page_bitmap[MAX_PAGES / 8];

static struct page_directory *current_dir = 0;

static void heap_init(void);
static struct heap_block *heap_find_best_fit(uint64_t size);
static void heap_split_block(struct heap_block *block, uint64_t size);
static void paging_invalidate(uint64_t virt);

static inline void *phys_to_virt(uint64_t phys) {
    return (void *)phys;
}

static inline uint64_t virt_to_phys(void *v) {
    return (uint64_t)v;
}

void mm_init(void) {
    kset_color(0x0F);
    kputs("  Initializing physical memory manager...\n");

    for (int i = 0; i < PAGES_TOTAL; i++) {
        pages[i].phys_addr = i * PAGE_SIZE;
        pages[i].present = 0;
        pages[i].writable = 0;
        pages[i].user = 0;
    }

    for (int i = 0; i < PAGES_TOTAL / 8; i++) {
        page_bitmap[i] = 0;
    }

    phys_region_count = 0;

    phys_regions[phys_region_count].base = 0x000000;
    phys_regions[phys_region_count].size = 0x100000;
    phys_regions[phys_region_count].type = 1;
    phys_regions[phys_region_count].used = 1;
    phys_region_count++;

    phys_regions[phys_region_count].base = 0x100000;
    phys_regions[phys_region_count].size = 0x100000;
    phys_regions[phys_region_count].type = 1;
    phys_regions[phys_region_count].used = 1;
    phys_region_count++;

    uint64_t map_end = (uint64_t)RAM_SIZE_GB * 1024ULL * 1024ULL * 1024ULL;
    if (map_end > (uint64_t)MAX_PHYS_PAGES * PAGE_SIZE) {
        map_end = (uint64_t)MAX_PHYS_PAGES * PAGE_SIZE;
    }

    for (uint64_t p = 0x200000; p < map_end; p += PAGE_SIZE) {
        int idx = p / PAGE_SIZE;
        if (idx < PAGES_TOTAL) {
            page_bitmap[idx / 8] |= (1 << (idx % 8));
            pages[idx].present = 1;
        }
    }

    phys_regions[phys_region_count].base = 0x200000;
    phys_regions[phys_region_count].size = map_end - 0x200000;
    phys_regions[phys_region_count].type = 2;
    phys_regions[phys_region_count].used = 0;
    phys_region_count++;

    kprintf("  Mapped %d MB of physical memory\n", (int)(map_end / (1024 * 1024)));
}

uint64_t mm_alloc_page(void) {
    for (int i = 0; i < PAGES_TOTAL; i++) {
        int byte = i / 8;
        int bit = i % 8;
        if (page_bitmap[byte] & (1 << bit)) {
            page_bitmap[byte] &= ~(1 << bit);
            pages[i].present = 1;
            return (uint64_t)i * PAGE_SIZE;
        }
    }
    return 0;
}

void mm_free_page(uint64_t phys) {
    int idx = phys / PAGE_SIZE;
    if (idx >= PAGES_TOTAL) return;
    page_bitmap[idx / 8] |= (1 << (idx % 8));
    pages[idx].present = 0;
}

static uint64_t *get_pte(uint64_t virt, int create) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt >> 12) & 0x1FF;

    uint64_t pml4e = current_dir->entries[pml4_idx];
    if (!(pml4e & PAGE_PRESENT)) {
        if (!create) return 0;
        uint64_t pt_phys = mm_alloc_page();
        if (!pt_phys) return 0;
        struct page_table *pdpt = (struct page_table *)phys_to_virt(pt_phys);
        for (int i = 0; i < PAGES_PER_TABLE; i++) pdpt->entries[i] = 0;
        current_dir->entries[pml4_idx] = pt_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        pml4e = current_dir->entries[pml4_idx];
    }

    uint64_t pdpt_phys = pml4e & ~0xFFFULL;
    uint64_t *pdpt = (uint64_t *)phys_to_virt(pdpt_phys);

    uint64_t pdpte = pdpt[pdpt_idx];
    if (!(pdpte & PAGE_PRESENT)) {
        if (!create) return 0;
        uint64_t pd_phys = mm_alloc_page();
        if (!pd_phys) return 0;
        uint64_t *pd = (uint64_t *)phys_to_virt(pd_phys);
        for (int i = 0; i < PAGES_PER_TABLE; i++) pd[i] = 0;
        pdpt[pdpt_idx] = pd_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        pdpte = pdpt[pdpt_idx];
    }

    uint64_t pd_phys = pdpte & ~0xFFFULL;
    uint64_t *pd = (uint64_t *)phys_to_virt(pd_phys);

    uint64_t pde = pd[pd_idx];
    if (!(pde & PAGE_PRESENT)) {
        if (!create) return 0;
        uint64_t pt_phys = mm_alloc_page();
        if (!pt_phys) return 0;
        uint64_t *pt = (uint64_t *)phys_to_virt(pt_phys);
        for (int i = 0; i < PAGES_PER_TABLE; i++) pt[i] = 0;
        pd[pd_idx] = pt_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        pde = pd[pd_idx];
    }

    uint64_t pt_phys = pde & ~0xFFFULL;
    uint64_t *pt = (uint64_t *)phys_to_virt(pt_phys);

    return &pt[pt_idx];
}

void *mm_map_page(uint64_t phys, uint64_t virt, uint64_t flags) {
    uint64_t *pte = get_pte(virt, 1);
    if (!pte) return 0;
    *pte = (phys & ~0xFFFULL) | (flags & 0xFFF) | PAGE_PRESENT;
    paging_invalidate(virt);
    return (void *)virt;
}

void *mm_unmap_page(uint64_t virt) {
    uint64_t *pte = get_pte(virt, 0);
    if (!pte || !(*pte & PAGE_PRESENT)) return 0;
    uint64_t phys = *pte & ~0xFFFULL;
    *pte = 0;
    paging_invalidate(virt);
    return (void *)phys;
}

void paging_switch_directory(struct page_directory *dir) {
    current_dir = dir;
    uint64_t phys = (uint64_t)dir;
    asm volatile("movq %0, %%cr3" : : "r"(phys) : "memory");
}

void paging_invalidate(uint64_t virt) {
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

static struct page_directory kernel_dir;

void paging_init(void) {
    kset_color(0x0F);
    kputs("  Initializing 4KB paging...\n");

    uint64_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 4) | (1 << 5);
    asm volatile("mov %0, %%cr4" : : "r"(cr4));

    heap_init();

    current_dir = &kernel_dir;

    kprintf("  Paging: identity-mapped first 2MB (2MB pages)\n");
    kprintf("  Paging: extended to %d MB with 2MB pages\n",
            (int)((uint64_t)RAM_SIZE_GB * 1024));
}

void mm_dump_page_table(uint64_t virt) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt >> 12) & 0x1FF;

    kset_color(0x07);
    kprintf("  VIRT 0x%llx:\n", (unsigned long long)virt);
    kprintf("    PML4[%d]\n", (int)pml4_idx);
    kprintf("    PDPT[%d]\n", (int)pdpt_idx);
    kprintf("    PD[%d]\n",   (int)pd_idx);
    kprintf("    PT[%d]\n",   (int)pt_idx);
}

void mm_print_stats(void) {
    int used = 0;
    int free = 0;
    for (int i = 0; i < PAGES_TOTAL; i++) {
        int byte = i / 8;
        int bit = i % 8;
        if (page_bitmap[byte] & (1 << bit)) free++;
        else used++;
    }

    kset_color(0x0F);
    kputs("Memory Manager Statistics:\n");
    kprintf("  Total pages: %d (%d KB)\n", PAGES_TOTAL, (int)(PAGES_TOTAL * 4));
    kprintf("  Used pages:  %d (%d KB)\n", used, used * 4);
    kprintf("  Free pages:  %d (%d KB)\n", free, free * 4);
    kprintf("  Regions:     %d\n", phys_region_count);
    kprintf("  Page size:   %d bytes\n", (int)PAGE_SIZE);
}

#define HEAP_BLOCK_MAGIC 0xDEADBEEF12345678ULL

struct heap_block {
    uint64_t magic;
    uint64_t size;
    uint8_t  is_free;
    uint8_t  reserved[7];
    struct heap_block *next;
    struct heap_block *prev;
};

static struct heap_block *heap_head = 0;
static uint64_t heap_total = 0;
static uint64_t heap_used = 0;
static uint64_t heap_free_bytes = 0;

void heap_init(void) {
    heap_head = (struct heap_block *)HEAP_START;
    heap_head->magic = HEAP_BLOCK_MAGIC;
    heap_head->size = HEAP_SIZE - sizeof(struct heap_block);
    heap_head->is_free = 1;
    heap_head->next = 0;
    heap_head->prev = 0;
    heap_total = HEAP_SIZE;
    heap_used = sizeof(struct heap_block);
    heap_free_bytes = heap_head->size;
}

static struct heap_block *heap_find_best_fit(uint64_t size) {
    struct heap_block *best = 0;
    struct heap_block *cur = heap_head;
    while (cur) {
        if (cur->is_free && cur->size >= size) {
            if (!best || cur->size < best->size) {
                best = cur;
                if (best->size == size) break;
            }
        }
        cur = cur->next;
    }
    return best;
}

static void heap_split_block(struct heap_block *block, uint64_t size) {
    if (block->size <= size + sizeof(struct heap_block) + 16) return;

    uint8_t *base = (uint8_t *)block;
    uint64_t old_size = block->size;

    struct heap_block *new_block = (struct heap_block *)(base + sizeof(struct heap_block) + size);
    new_block->magic = HEAP_BLOCK_MAGIC;
    new_block->size = old_size - size - sizeof(struct heap_block);
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next) {
        block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;

    heap_free_bytes = new_block->size;
}

void *kmalloc_aligned(uint64_t size, uint64_t alignment);

void heap_print_blocks(void) {
    kset_color(0x0F);
    kputs("Heap Status:\n");
    kprintf("  Total:    %d bytes\n", (int)heap_total);
    kprintf("  Used:     %d bytes\n", (int)heap_used);
    kprintf("  Free:     %d bytes\n", (int)heap_free_bytes);

    int count = 0;
    struct heap_block *cur = heap_head;
    while (cur) {
        kprintf("  Block %d: addr=0x%llx size=%d %s\n",
                count,
                (unsigned long long)(uint64_t)cur,
                (int)cur->size,
                cur->is_free ? "FREE" : "USED");
        count++;
        cur = cur->next;
    }
}
