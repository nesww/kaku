#pragma once

#include <lib/core.h>
#include <stdint.h>

#define ALLOC_MIN_BYTES_BLOCK 16
#define ALLOC_MAX_BYTES_BLOCK MB(4)
#define ALLOC_ORDERS 19

typedef struct {
    void *free_blocks[ALLOC_ORDERS];
    void *heap_start;
    uint32_t heap_size;
} k_heap;

typedef struct free_block {
   struct free_block *next;
} free_block;

void kheap_init();
void *kheap_alloc(uint32_t size);
void kheap_free(void *ptr);

void *kheap_get_start(void);
uint32_t kheap_get_size(void);
