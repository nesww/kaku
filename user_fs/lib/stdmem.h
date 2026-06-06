#pragma once

#include "stdutils.h"
#include <stdint.h>

static inline void *memcpy(void *dst, const void *src, uint32_t n) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dst;
}

static inline void memset(void *dst, uint8_t val, uint32_t n) {
    uint8_t *d = (uint8_t*)dst;
    while (n--) *d++ = val;
}

__attribute__((always_inline))
static inline void *malloc(uint32_t size) {
    TODO("malloc");
}

__attribute__((always_inline))
static inline void free(void *ptr) {
    TODO("free");
}

__attribute__((always_inline))
static inline void *realloc(void *ptr, uint32_t old, uint32_t new) {
    TODO("realloc");
}
