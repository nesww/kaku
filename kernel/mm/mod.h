#include <stdint.h>

/*===========================================================================
 * mm — memory management public API
 *
 * selective includes: define MM_<SUBMOD>_IPL before including
 * this header to pull in a specific subsystem.
 *
 *   #define MM_HEAP_IPL
 *   #define MM_FRAME_IPL
 *   #define MM_PAGING_IPL
 *   #include <mm/mod.h>
 *
 * or define MM_ALL_IPL to include everything.
 *===========================================================================*/

#ifdef MM_ALL_IPL
#  define MM_HEAP_IPL
#  define MM_FRAME_IPL
#  define MM_PAGING_IPL
#endif

/*===========================================================================
 * mm/heap — Kernel heap allocator
 *===========================================================================*/
#if defined(MM_HEAP_IPL) && !defined(MM_HEAP_IPL_GUARD)
#  define MM_HEAP_IPL_GUARD
#  include "src/heap.h"

static inline void    *mm_alloc(uint32_t size)          { return kheap_alloc(size); }
static inline void     mm_free(void *ptr)               { kheap_free(ptr); }
static inline uint32_t mm_heap_get_size(void)           { return kheap_get_size(); }
#endif

/*===========================================================================
 * mm/frame — Physical frame allocator
 *===========================================================================*/
#if defined(MM_FRAME_IPL) && !defined(MM_FRAME_IPL_GUARD)
#  define MM_FRAME_IPL_GUARD
#  include "src/frame.h"

static inline void *mm_alloc_frame(void)                { return fa_alloc(); }
static inline void  mm_free_frame(uint32_t addr)        { fa_free(addr); }
#endif

/*===========================================================================
 * mm/paging — Virtual memory / paging
 *===========================================================================*/
#if defined(MM_PAGING_IPL) && !defined(MM_PAGING_IPL_GUARD)
#  define MM_PAGING_IPL_GUARD
#  include "src/paging.h"
#  include "src/mem.h"

static inline void *mm_paging_create_pd(void)           { return paging_create_pd(); }
static inline void  mm_paging_switch(void *pd)          { paging_switch((page_directory *)pd); }
static inline void  mm_paging_map(void *pd, uint32_t paddr, uint32_t vaddr, uint8_t flags) {
    paging_map((page_directory *)pd, paddr, vaddr, flags);
}
static inline const void *mm_paging_get_kernel_pd(void) { return paging_get_kernel_pd(); }
#endif

/*===========================================================================
 * Unified init — requires MM_ALL_IPL
 *===========================================================================*/
#if defined(MM_ALL_IPL) && !defined(MM_ALL_IPL_GUARD)
#  define MM_ALL_IPL_GUARD
static inline void mm_init(void) {
    kheap_init();
    fa_init();
    paging_kernel_init();
}
#endif
