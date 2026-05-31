#include "alloc.h"

#include "panic/panic.h"
#include "mem/mem.h"

#include <stdint.h>

static k_heap kernel_heap = {0};
static uint8_t heap_initialized = FALSE;
extern uint32_t kernel_end;


void kheap_init() {
    for (uint32_t i = 0; i < ALLOC_ORDERS - 1; ++i) {
        kernel_heap.free_blocks[i] = 0;
    }
    free_block *initial = (free_block*)&kernel_end;
    initial->next = 0;
    kernel_heap.free_blocks[ALLOC_ORDERS - 1] = initial;
    kernel_heap.heap_start = &kernel_end;
    mmap_entry usable_entry = mmap_get_usable_entry();
    kernel_heap.heap_size = KMIN(usable_entry.base_addr + usable_entry.region_len - (uint32_t)(kernel_heap.heap_start), MB(32));
    heap_initialized = TRUE;
}

uint8_t __order_for_size(uint32_t size) {
    uint8_t order = 0;
    uint32_t evaluated_order_bytes = ALLOC_MIN_BYTES_BLOCK;
    while (KMAX(size, evaluated_order_bytes) == size) {
        evaluated_order_bytes = evaluated_order_bytes << 1;
        ++order;
    }
    return order;
}

uint32_t __size_for_order(uint8_t order) {
    return ALLOC_MIN_BYTES_BLOCK << (order);
}

void *kheap_alloc(uint32_t size) {
    if (!heap_initialized) kernel_panic("HEAP_NOT_INITIALIZED: tried to allocate to the heap without initializing the heap!");
    uint8_t order = __order_for_size(size);
    int8_t first_available_order = -1;
    for (uint32_t i = order; i < ALLOC_ORDERS; ++i) {
        if (kernel_heap.free_blocks[i] != 0) {
            first_available_order = i;
            break;
        }
    }
    if (first_available_order == -1) {
        kernel_panic("HEAP_OUT_OF_MEMORY: kernel heap is out of memory, cannot allocate!");
        __builtin_unreachable();
    }
    void *ptr = kernel_heap.free_blocks[first_available_order];
       kernel_heap.free_blocks[first_available_order] = ((free_block*)ptr)->next;
       for (int8_t i = first_available_order; i > order; --i) {
           uint32_t half = __size_for_order(i - 1);
           free_block *buddy = (free_block*)((uint32_t)ptr + half);
           buddy->next = kernel_heap.free_blocks[i - 1];
           kernel_heap.free_blocks[i - 1] = buddy;
       }
       uint8_t *header = (uint8_t*)ptr;
       *header = order;
       return (void*)((uint32_t)ptr + 1);
}

void kheap_free(void *ptr) {
    if (!ptr) return;
    //extract the order from the header
    uint8_t current_order = *((uint8_t*)ptr - 1);
    uint32_t current_addr = (uint32_t)(ptr - 1);
    while(current_order < ALLOC_ORDERS - 1) {
        uint32_t offset_from_start = current_addr - (uint32_t)kernel_heap.heap_start;
        uint32_t buddy_offset = offset_from_start^ __size_for_order(current_order);
        uint32_t buddy_addr = (uint32_t)kernel_heap.heap_start + buddy_offset;

        free_block *current_block = kernel_heap.free_blocks[current_order];
        free_block **previous_block = (free_block**)&kernel_heap.free_blocks[current_order];
        uint8_t found = FALSE;
        while (current_block) {
            if (current_block == (free_block*)buddy_addr) {
                *previous_block = current_block->next;
                found = TRUE;
                break;
            }
            previous_block = &current_block->next;
            current_block = current_block->next;
        }
        if (found) {
            if (buddy_addr < current_addr) {
                current_addr = buddy_addr;
            }
            current_order++;
        } else {
            break;
        }
    }

    free_block *block = (free_block*)(current_addr);
    block->next = kernel_heap.free_blocks[current_order];
    kernel_heap.free_blocks[current_order] = (void*)current_addr;
}

void *kheap_get_start(void) {
    return kernel_heap.heap_start;
}
uint32_t kheap_get_size(void) {
    return kernel_heap.heap_size;
}
