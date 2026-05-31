#ifndef STDMEM_H
#define STDMEM_H

#include "alloc/alloc.h"

static inline void *kmemcpy(void *dst, const void *src, uint32_t n) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dst;
}

static inline void *kmalloc(uint32_t size) {
    return kheap_alloc(size);
}

static inline void kfree(void* ptr) {
    return kheap_free(ptr);
}

static inline void *krealloc(void* ptr, uint32_t old_size, uint32_t new_size) {
    void *new_ptr = kmalloc(new_size);
    kmemcpy(new_ptr, ptr, old_size);
    kfree(ptr);
    return new_ptr;
}

#endif //STDMEM_H
