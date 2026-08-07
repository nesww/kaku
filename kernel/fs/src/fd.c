#define LIB_ARRAY_IPL
#define LIB_MEM_IPL

#include <lib/mod.h>
#include <proc/mod.h>

#include "fd.h"

static dynarray* __fd_table = {0};

void fd_init_table(void) {
    __fd_table = dynarray_new(512);
}


uint32_t fd_new(vfs_node *node, uint8_t flags) {
    fd_table_entry *entry = kmalloc(sizeof(fd_table_entry));
    entry->node = node;
    entry->ref_count = 0;
    entry->write_lock = (flags & FD_FLAGS_RO) ? FALSE : TRUE;
    return fd_add(entry);
}

uint32_t fd_add(fd_table_entry *entry) {
    for (uint32_t i = 0; i < __fd_table->count; ++i) {
        fd_table_entry *e = dynarray_get(__fd_table, i);
        if (!e) {
            ((void**)__fd_table->data)[i] = entry;
            return i;
        }
    }
    dynarray_add(__fd_table, entry);
    return __fd_table->count - 1;
}

fd_table_entry *fd_get(uint32_t fd_global) {
    return dynarray_get(__fd_table, fd_global);
}

int fd_find_by_node(vfs_node *node) {
    for (uint32_t i = 0; i < __fd_table->count; ++i) {
        fd_table_entry *entry = dynarray_get(__fd_table, i);
        if (entry && entry->node->vnode_id == node->vnode_id) return i;
    }
    return -1;
}

void fd_remove(uint32_t fd_global) {
    fd_table_entry *entry = dynarray_get(__fd_table, fd_global);
    if (!entry) return;

    if (!--entry->ref_count) {
        kfree(entry);
        ((void**)__fd_table->data)[fd_global] = 0;
    };
}
