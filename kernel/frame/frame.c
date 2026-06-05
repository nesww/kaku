#include <mem/mem.h>
#include <panic/panic.h>
#include <lib/stdmem.h>
#include <lib/core.h>

#include "frame.h"

static uint8_t *fa_bitmap;
static uint32_t fa_bitmap_size = 0;
static uint8_t fa_initialized = FALSE;

void __fa_init_alloc_bitmap(void) {
    mmap_entry *entries = mem_get_mmap_entries();
    uint32_t max_end_addr = (entries[0].base_addr + entries[0].region_len);
    for (uint32_t i = 0; i < *(uint32_t*)MMAP_COUNTER_ADDR; ++i) {
        mmap_entry entry = entries[i];
        uint32_t end_addr = entry.base_addr + entry.region_len;
        if (end_addr > max_end_addr) {
            max_end_addr = end_addr;
        }
    }
    fa_bitmap_size = (max_end_addr / 4096 / 8);
    fa_bitmap = kmalloc(fa_bitmap_size);
}

void fa_init(void) {
    __fa_init_alloc_bitmap();
    for (uint32_t i = 0; i < fa_bitmap_size; ++i) {
            fa_bitmap[i] = 0xFF;
    }

    mmap_entry usable_entry = mmap_get_usable_entry();
    uint32_t start_fa_usable = (uint32_t)&kernel_phys_end + kheap_get_size();
    uint32_t frame_index_start = start_fa_usable / 4096;
    uint32_t end_fa_usable = usable_entry.base_addr + usable_entry.region_len;
    uint32_t frame_index_end = end_fa_usable / 4096;

    for (uint32_t i = frame_index_start; i < frame_index_end; ++i) {
        uint32_t byte = i / 8;
        uint32_t bit = i % 8;
        fa_bitmap[byte] &= ~(1 << bit);
    }
    fa_initialized = TRUE;
}

void *fa_alloc() {
    if (!fa_initialized) kernel_panic("FRAME_ALLOCATOR_NOT_INITIALIZED: tried to allocate on the frame allocator without initializing it!");
    uint32_t frame_byte_index = 0;
    uint8_t frame_bit_index = 0;
    uint32_t i = 0;
    uint8_t free_found = FALSE;
    while(i< fa_bitmap_size && !free_found) {
        uint8_t byte = fa_bitmap[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (!(byte & (1 << j))) {
                frame_byte_index = i;
                frame_bit_index = j;
                free_found = TRUE;
                break;
            }
        }
        i++;
    }
    if (free_found) {
        uint32_t frame_index = frame_byte_index * 8 + frame_bit_index;
        fa_bitmap[frame_byte_index] |= (1<< frame_bit_index);
        return (void*)(frame_index*4096);
    }
    kernel_panic("could not allocate for frame allocator: no free frame available!");
    __builtin_unreachable();
}

void fa_free(uint32_t addr) {
    if (!fa_initialized) kernel_panic("FRAME_ALLOCATOR_NOT_INITIALIZED: tried to free on the frame allocator without initializing it!");
    uint32_t frame_index = addr / 4096;
    uint32_t byte = frame_index / 8;
    uint8_t bit = frame_index % 8;
    fa_bitmap[byte] &= ~(1 << bit);
}
