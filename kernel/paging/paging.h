#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGING_FLAGS_PRESENT     0x1
#define PAGING_FLAGS_READWRITE   0x2
#define PAGING_FLAGS_KERNEL_ONLY 0x4

/*present, writable, kernel-only = 0b00000011 = 0x3*/
#define PAGING_PD_ENTRY_FLAGS_KERNEL_ONLY PAGING_FLAGS_PRESENT | PAGING_FLAGS_READWRITE | PAGING_FLAGS_KERNEL_ONLY
#define PAGING_LOAD_CR3(addr) \
    __asm__ volatile("mov %0, %%cr3": : "r"(addr))

#define PAGING_ENABLE()             \
    __asm__ volatile(               \
        "mov %%cr0, %%eax\n"        \
        "or $0x80000000, %%eax\n"   \
        "mov %%eax, %%cr0\n"        \
        : : : "eax"                 \
    )

typedef uint32_t pd_entry;
typedef uint32_t pt_entry;

typedef struct {
    uint32_t entries[1024];
} page_directory;

typedef struct {
    uint32_t entries[1024];
} page_table;

page_directory *paging_kernel_init(void);
const page_directory *paging_get_kernel_pd(void);

void paging_map(page_directory *pd, uint32_t paddr, uint32_t vaddr, uint8_t flags);
page_directory *paging_create_pd(void);
void paging_switch(page_directory *pd);
#endif //PAGING_H
