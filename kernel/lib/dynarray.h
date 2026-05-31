#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdint.h>
#include "stdmem.h"

typedef struct {
    void *data;
    uint32_t count;
    uint32_t capacity;
} dynarray;

static inline dynarray *dynarray_new(uint32_t size) {
    dynarray *da = (dynarray*)kmalloc(sizeof(dynarray));
    da->data = kmalloc(size * sizeof(void*));
    da->count = 0;
    da->capacity = size;
    return da;
}

static inline void dynarray_add(dynarray *da, void *elt) {
    if(da->count + 1 > da->capacity) {
        da->data = krealloc(da->data, da->capacity * sizeof(void*), da->capacity*2 * sizeof(void*));
        da->capacity*=2;
    }
    ((void**)da->data)[da->count] = elt;
    da->count++;
}

static inline void *dynarray_remove_last(dynarray *da) {
    if (da->count > 0) {
        void *elt = ((void**)da->data)[da->count-1];
        da->count--;
        return elt;
    }
    return 0;
}

static inline void *dynarray_get(dynarray *da, uint32_t index) {
    if (index >= da->count) return 0;
    return ((void**)da->data)[index];
}

static inline void dynarray_free(dynarray *da) {
    if(!da)
        return;
    kfree(da->data);
    kfree(da);
}

static inline void dynarray_free_deep(dynarray *da, void(*free_callback)(void*)) {
    if (!da)
        return;

    for (uint32_t i = 0; i < da->count; ++i) {
        free_callback(((void**)da->data)[i]);
    }
    dynarray_free(da);
}

#endif //DYNARRAY_H
