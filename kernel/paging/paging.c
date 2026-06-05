#include <alloc/alloc.h>
#include <frame/frame.h>
#include <panic/panic.h>
#include <lib/core.h>
#include <lib/stdmem.h>

#include "paging.h"

static page_directory *kernel_pd;
static uint8_t paging_initialized = FALSE;

page_directory *paging_kernel_init(void) {
    uint32_t pd_phys = (uint32_t)fa_alloc();
    kernel_pd = (page_directory*)(PHYS_TO_VIRT(pd_phys));
    for (uint32_t i = 0; i < 1024; ++i) {
        kernel_pd->entries[i] = 0;
    }
    paging_initialized = TRUE;

    mmap_entry usable_entry = mmap_get_usable_entry();
    uint32_t phys_end = usable_entry.base_addr + usable_entry.region_len;
    uint32_t phys_end_aligned = (phys_end + 0xfff) & ~0xfff;
    for (uint32_t addr = 0; addr < phys_end_aligned; addr += 4096) {
        paging_map(kernel_pd, addr, PHYS_TO_VIRT(addr), PAGING_PD_ENTRY_FLAGS_KERNEL_ONLY);
    }

    PAGING_LOAD_CR3(VIRT_TO_PHYS(kernel_pd));

    return kernel_pd;
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
    page_table *pt = (page_table *)PHYS_TO_VIRT(pd->entries[pd_index] & 0xFFFFF000);
    pt->entries[pt_index] = paddr | flags;
}


page_directory *paging_create_pd(void) {
    if (!paging_initialized) kernel_panic("PAGING_NOT_INITIALIZED: tried to create a page directory for a processus without initializing kernel paging");

    uint32_t pd_phys = (uint32_t)fa_alloc();
    page_directory *pd =(page_directory*)PHYS_TO_VIRT(pd_phys);
    for (uint32_t i = 0; i < 1024; ++i) {
        pd->entries[i] = 0;
    }

    for (uint32_t i = 0; i < 1024; i++) {
        if (kernel_pd->entries[i] != 0) {
            uint8_t flags = kernel_pd->entries[i] & 0xFFF;
            page_table *kernel_pt = (page_table*)PHYS_TO_VIRT(kernel_pd->entries[i] & 0xFFFFF000);

            uint32_t new_pt_phys = (uint32_t)fa_alloc();
            page_table *new_pt = (page_table*)PHYS_TO_VIRT(new_pt_phys);

            kmemcpy(new_pt, kernel_pt, sizeof(page_table));
            pd->entries[i] = new_pt_phys | flags;
        }
    }
    return pd;
}

void paging_switch(page_directory *pd) {
    if (!paging_initialized) kernel_panic("PAGING_NOT_INITIALIZED: tried to switch paging without initializing kernel paging");

    PAGING_LOAD_CR3(VIRT_TO_PHYS(pd));
}
