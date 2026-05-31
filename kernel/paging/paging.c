#include "paging.h"
#include "frame/frame.h"
#include "panic/panic.h"
#include "mem/mem.h"
#include "lib/core.h"

static page_directory *kernel_pd;
static uint8_t paging_initialized = FALSE;

page_directory *paging_kernel_init(void) {
    kernel_pd = (page_directory*)fa_alloc();
    for (uint32_t i = 0; i < 1024; ++i) {
        kernel_pd->entries[i] = 0;
    }
    paging_initialized = TRUE;

    mmap_entry entry = mmap_get_usable_entry();
    for (uint32_t addr = 0; addr < entry.base_addr + entry.region_len; addr+=4096) {
        paging_map(kernel_pd, addr, addr, PAGING_PD_ENTRY_FLAGS_KERNEL_ONLY);
    }

    PAGING_LOAD_CR3(kernel_pd);
    PAGING_ENABLE();
}

const page_directory *paging_get_kernel_pd(void) {
    if (!paging_initialized) kernel_panic("PAGING_NOT_INITIALIZED: tried getting kernel page directory without kernel paging initialized");
    return kernel_pd;
}

void paging_map(page_directory *pd, uint32_t paddr, uint32_t vaddr, uint8_t flags) {
    if (!paging_initialized) kernel_panic("PAGING_NOT_INITIALIZED: tried mapping to a page directory without initializing kernel paging");

    uint32_t pd_index = vaddr >> 22;
    if(pd->entries[pd_index] == 0) {
        pd->entries[pd_index] = (uint32_t)fa_alloc() | flags;
    }
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;
    page_table *pt = (page_table *)(pd->entries[pd_index] & 0xFFFFF000);
    pt->entries[pt_index] = paddr | flags;
}


page_directory *paging_create_pd(void) {
    if (!paging_initialized) kernel_panic("PAGING_NOT_INITIALIZED: tried to create a page directory for a processus without initializing kernel paging");

    page_directory *pd =(page_directory*)fa_alloc();
    for (uint32_t i = 0; i < 1024; ++i) {
        pd->entries[i] = 0;
    }

    for (uint32_t i = 0; i < 1024; i++) {
        if (kernel_pd->entries[i] != 0) {
            pd->entries[i] = kernel_pd->entries[i];
        }
    }
    return pd;
}

void paging_switch(page_directory *pd) {
    if (!paging_initialized) kernel_panic("PAGING_NOT_INITIALIZED: tried to switch paging without initializing kernel paging");

    PAGING_LOAD_CR3(pd);
}
