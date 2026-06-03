#include "tss.h"
#include "lib/core.h"
#include "panic/panic.h"
#include <stdint.h>

static tss_t tss = {0};
static uint8_t tss_initialized = FALSE;

void tss_init(void) {
    tss.ss0 = 0x10;
    tss.esp0 = 0x9FF00;
    tss.iomap_base = sizeof(tss_t);
    tss_initialized = TRUE;
}

void tss_install(void) {
    if (!tss_initialized) kernel_panic("TSS_NOT_INITIALIZED: tried installing TSS in GDT before it was initialized");
    struct { uint16_t limit; uint32_t base; } __attribute__((packed)) gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));
    uint64_t *gdt = (uint64_t*)gdtr.base;
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss_t) - 1;
    gdt[5] = 0;
    gdt[5] |= (uint64_t)(limit & 0xFFFF);
    gdt[5] |= (uint64_t)(base & 0xFFFFFF) << 16;
    gdt[5] |= (uint64_t)0x89 << 40;
    gdt[5] |= (uint64_t)((limit >> 16) & 0xF) << 48;
    gdt[5] |= (uint64_t)((base >> 24) & 0xFF) << 56;
    asm volatile("ltr %%ax" :: "a"(0x28));
}
