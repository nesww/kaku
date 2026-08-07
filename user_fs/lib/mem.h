#pragma once

#include <stdint.h>
#include <sys/syscalls.h>

/*
 * userspace memory allocator
 *
 * - small allocations  : served from an arena backed by the process brk
 *                         (SYS_SBRK), managed with segregated free lists.
 * - mid allocations    : single first-fit free list over the same arena.
 * - large allocations  : >= MEM_MMAP_THRESHOLD, served directly by SYS_MMAP
 *                         and returned with SYS_MUNMAP.
 *
 * all symbols are `static` so the header can be included from several TUs of
 * the same program without link clashes; a program effectively gets a single
 * allocator instance.
 */

#define MEM_ALIGN          16
#define MEM_MMAP_THRESHOLD (64 * 1024)
#define MEM_NUM_CLASSES    10           /* 16 .. 8192 */

#define MEM_FREE 0x1
#define MEM_USED 0x2
#define MEM_MMAP 0x4

typedef struct mem_block {
    uint32_t            size;
    uint32_t            flags;
    struct mem_block   *next;
    uint32_t            pad;
} mem_block;

#define MEM_HEADER  ((uint32_t)sizeof(mem_block))

static mem_block *mem_free_lists[MEM_NUM_CLASSES];
static mem_block *mem_large_free;

/* --- syscall wrappers -------------------------------------------------- */

static uint32_t mem_sbrk(int32_t increment) {
    uint32_t old_break;
    SYS_SBRK(increment, old_break);
    return old_break;
}

static void *mem_mmap(uint32_t size) {
    uint32_t base;
    SYS_MMAP(size, base);
    return (void*)base;
}

static void mem_munmap(void *addr, uint32_t size) {
    SYS_MUNMAP((uint32_t)addr, size);
}

/* --- helpers ----------------------------------------------------------- */

static uint32_t mem_block_size_for_class(int ci) {
    return (uint32_t)MEM_ALIGN << ci;   /* 16, 32, ... , 8192 */
}

/* smallest class whose block size >= `size`, or -1 if it is a large block */
static int mem_class_index(uint32_t size) {
    uint32_t bs = MEM_ALIGN;
    for (int i = 0; i < MEM_NUM_CLASSES; ++i) {
        if (bs >= size) return i;
        bs <<= 1;
    }
    return -1;
}

/* carve a fresh arena chunk into blocks of the requested class */
static void mem_arena_extend_small(int ci) {
    uint32_t bs = mem_block_size_for_class(ci);
    uint32_t chunk = bs * 8;
    uint32_t base = mem_sbrk((int32_t)chunk);
    if (base == 0xFFFFFFFF) return;

    for (int i = 0; i < 8; ++i) {
        mem_block *b = (mem_block*)(base + (uint32_t)i * bs);
        b->size  = bs;
        b->flags = MEM_FREE;
        b->next  = mem_free_lists[ci];
        mem_free_lists[ci] = b;
    }
}

static void *mem_alloc_small(int ci) {
    if (!mem_free_lists[ci]) mem_arena_extend_small(ci);
    if (!mem_free_lists[ci]) return 0;   /* out of memory */
    mem_block *b = mem_free_lists[ci];
    mem_free_lists[ci] = b->next;
    b->flags = MEM_USED;
    return (char*)b + MEM_HEADER;
}

static void *mem_alloc_large(uint32_t total) {
    mem_block **pp = &mem_large_free;
    while (*pp) {
        mem_block *b = *pp;
        if (b->size >= total) {
            *pp = b->next;
            b->flags = MEM_USED;
            return (char*)b + MEM_HEADER;
        }
        pp = &b->next;
    }

    uint32_t base = mem_sbrk((int32_t)total);
    if (base == 0xFFFFFFFF) return 0;
    mem_block *b = (mem_block*)base;
    b->size  = total;
    b->flags = MEM_USED;
    return (char*)b + MEM_HEADER;
}

/* --- public API -------------------------------------------------------- */

static void *malloc(uint32_t size) {
    if (size == 0) size = 1;
    uint32_t total = size + MEM_HEADER;

    if (total >= MEM_MMAP_THRESHOLD) {
        void *base = mem_mmap(total);
        if (!base) return 0;
        mem_block *b = (mem_block*)base;
        b->size  = total;
        b->flags = MEM_MMAP;
        return (char*)base + MEM_HEADER;
    }

    int ci = mem_class_index(total);
    if (ci >= 0) return mem_alloc_small(ci);
    return mem_alloc_large(total);
}

static void free(void *ptr) {
    if (!ptr) return;

    mem_block *b = (mem_block*)((char*)ptr - MEM_HEADER);
    if (b->flags & MEM_MMAP) {
        mem_munmap((char*)b, b->size);
        return;
    }

    b->flags = MEM_FREE;
    int ci = mem_class_index(b->size);
    if (ci >= 0) {
        b->next = mem_free_lists[ci];
        mem_free_lists[ci] = b;
    } else {
        b->next = mem_large_free;
        mem_large_free = b;
    }
}

static void *calloc(uint32_t count, uint32_t size) {
    uint32_t bytes = count * size;
    void *p = malloc(bytes);
    if (!p) return 0;
    for (uint32_t i = 0; i < bytes; ++i) ((char*)p)[i] = 0;
    return p;
}

static void *realloc(void *ptr, uint32_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return 0; }

    mem_block *b = (mem_block*)((char*)ptr - MEM_HEADER);
    uint32_t old_usable = b->size - MEM_HEADER;
    if (old_usable >= size) return ptr;

    void *np = malloc(size);
    if (!np) return 0;
    for (uint32_t i = 0; i < old_usable; ++i) ((char*)np)[i] = ((char*)ptr)[i];
    free(ptr);
    return np;
}
