#include "mm.h"
#include <libk.h>

static int mm_initialized = 0;

void mm_start(void) {
    kset_color(0x0F);
    kputs("[mm] Starting memory manager...\n");

    mm_init();
    paging_init();
    mm_print_stats();

    kset_color(0x0F);
    kputs("[mm] Memory manager ready\n");

    mm_initialized = 1;
}

int mm_is_ready(void) {
    return mm_initialized;
}
