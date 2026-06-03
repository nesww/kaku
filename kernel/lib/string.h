#ifndef KCORE_STRING
#define KCORE_STRING
#include <stdint.h>
#include "dynarray.h"
#include "lib/stdmem.h"

static inline uint32_t kstrlen (const char *str) {
    uint32_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

static inline uint32_t kstrcmp(const char *first, const char *second) {
    while(*first && *first == *second) {first++; second++;};
    return *first - *second;
}

static inline dynarray *kstrsplit(const char *s, char delim) {
    uint32_t len = kstrlen(s);
    dynarray *da = dynarray_new(8);

    uint32_t segment_start_index = 0;
    for (uint32_t i = 0; i < len; ++i) {
        if (s[i] == delim) {
            uint8_t *segment = kmalloc(i - segment_start_index + 1);
            kmemcpy(segment, s + segment_start_index, i - segment_start_index);
            segment[i - segment_start_index] = '\0';
            dynarray_add(da, segment);
            segment_start_index = i + 1;
        }
    }
    if (segment_start_index < len) {
        uint8_t *segment = kmalloc(len - segment_start_index + 1);
        kmemcpy(segment, s + segment_start_index, len - segment_start_index);
        segment[len - segment_start_index] = '\0';
        dynarray_add(da, segment);
    }
    return da;
}

static inline char *kstrjoin(dynarray *strs, char delim) {
    uint32_t strs_len = 0;
    for (uint32_t i = 0; i < strs->count; ++i) {
        strs_len += kstrlen((char*)dynarray_get(strs, i));
    }
    char *result = kmalloc(strs_len + strs->count);
    uint32_t string_cursor = 0;
    for (uint32_t i = 0; i < strs->count; ++i) {
        char* str = (char*)dynarray_get(strs, i);
        uint32_t str_len = kstrlen(str);
        kmemcpy(result + string_cursor, str, str_len);
        string_cursor += kstrlen(str);
        if (i < strs->count - 1) result[string_cursor++] = delim;
    }
    result[string_cursor] = '\0';
    return result;
}

static inline void kstrcpy(char *dst, const char *src) {
    while((*dst++ = *src++));
}

#endif // KCORE_STRING
