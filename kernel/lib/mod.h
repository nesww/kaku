#pragma once

#include <stdint.h>
#include <stdarg.h>

/*===========================================================================
 * lib — standard library public API
 *
 * selective includes: define LIB_<SUBMOD>_IPL before including
 * this header to pull in a specific subsystem.
 *
 *   #define LIB_MEM_IPL
 *   #define LIB_STR_IPL
 *   #define LIB_PRINT_IPL
 *   #define LIB_ARRAY_IPL
 *   #define LIB_MATH_IPL
 *   #include <lib/mod.h>
 *
 * or define LIB_ALL_IPL to include everything.
 *===========================================================================*/

#ifdef LIB_ALL_IPL
#  define LIB_MEM_IPL
#  define LIB_STR_IPL
#  define LIB_PRINT_IPL
#  define LIB_ARRAY_IPL
#  define LIB_MATH_IPL
#endif

/*===========================================================================
 * common macros (always available)
 *===========================================================================*/
#include "src/core.h"

/*===========================================================================
 * lib/mem — Memory operations
 *===========================================================================*/
#ifdef LIB_MEM_IPL
#  include "src/stdmem.h"

static inline void *lib_memcpy(void *dst, const void *src, uint32_t n)  { return kmemcpy(dst, src, n); }
static inline void *lib_memset(void *dst, uint8_t val, uint32_t n)      { kmemset(dst, val, n); return dst; }
static inline void *lib_malloc(uint32_t size)                           { return kmalloc(size); }
static inline void  lib_free(void *ptr)                                 { kfree(ptr); }
static inline void *lib_realloc(void *ptr, uint32_t old_size, uint32_t new_size) {
    return krealloc(ptr, old_size, new_size);
}
#endif

/*===========================================================================
 * lib/str — String operations
 *===========================================================================*/
#ifdef LIB_STR_IPL
#  include "src/string.h"

static inline uint32_t lib_strlen(const char *s)                        { return kstrlen(s); }
static inline int      lib_strcmp(const char *a, const char *b)         { return kstrcmp(a, b); }
static inline void     lib_strcpy(char *dst, const char *src)           { kstrcpy(dst, src); }
static inline dynarray *lib_strsplit(const char *s, char delim)         { return kstrsplit(s, delim); }
static inline char    *lib_strjoin(dynarray *parts, char delim)         { return kstrjoin(parts, delim); }
#endif

/*===========================================================================
 * lib/print — Printf utilities
 *===========================================================================*/
#ifdef LIB_PRINT_IPL
#  include "src/print.h"

static inline void lib_kvprintf(void (*putc_fn)(char), void (*newline_fn)(void),
                                const char *fmt, va_list args) {
    kvprintf_to(putc_fn, newline_fn, fmt, args);
}

static inline void lib_kvsnprintf(char *buf, uint32_t size, const char *fmt, va_list args) {
    kvsnprintf(buf, size, fmt, args);
}
#endif

/*===========================================================================
 * lib/array — Dynamic array
 *===========================================================================*/
#ifdef LIB_ARRAY_IPL
#  include "src/dynarray.h"

static inline dynarray *lib_array_new(uint32_t capacity)                { return dynarray_new(capacity); }
static inline void      lib_array_add(dynarray *da, void *elt)          { dynarray_add(da, elt); }
static inline void     *lib_array_get(dynarray *da, uint32_t index)     { return dynarray_get(da, index); }
static inline void      lib_array_remove(dynarray *da, uint32_t index)  { dynarray_remove(da, index); }
static inline void     *lib_array_remove_last(dynarray *da)             { return dynarray_remove_last(da); }
static inline void      lib_array_free(dynarray *da)                    { dynarray_free(da); }
static inline void      lib_array_free_deep(dynarray *da, void (*free_fn)(void*)) {
    dynarray_free_deep(da, free_fn);
}
#endif

/*===========================================================================
 * lib/math — Math utilities
 *===========================================================================*/
#ifdef LIB_MATH_IPL
#  include "src/math.h"
#endif
