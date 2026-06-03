#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t prev_tss;
    uint32_t esp0; // kernel stack pointer
    uint32_t ss0;  // kernel stack segment
    uint32_t esp1; //ring1->useless
    uint32_t ss1;  //ring1->useless
    uint32_t esp2; //ring2->useless
    uint32_t ss2;  //ring2->useless
    uint32_t cr3;  // process page directory
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} tss_t;

void tss_init(void);
void tss_install(void);
