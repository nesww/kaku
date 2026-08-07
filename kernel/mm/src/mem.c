#include "mem.h"

mmap_entry mmap_get_usable_entry(void) {
    mmap_entry *entries = mem_get_mmap_entries();
    for (uint32_t i = 0; i < *(uint32_t*)MMAP_COUNTER_ADDR; ++i) {
        mmap_entry entry = entries[i];
        if (entry.region_type == MMAP_REGTYPE_USABLE && entry.base_addr >= 0x00100000) {
            return entry;
        }
    }
    return (mmap_entry){0};
}
