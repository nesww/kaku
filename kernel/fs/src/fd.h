#pragma once

#include "vfs.h"

#define FD_FLAGS_RO 0x1
#define FD_FLAGS_WO 0x2
#define FD_FLAGS_RW 0x4


typedef struct {
    vfs_node *node;
    uint32_t ref_count;
    uint8_t write_lock;
} fd_table_entry;

void fd_init_table(void);

uint32_t fd_new(vfs_node *node, uint8_t flags);
uint32_t fd_add(fd_table_entry *entry);
fd_table_entry *fd_get(uint32_t fd_global);
int fd_find_by_node(vfs_node *node);
void fd_remove(uint32_t fd_global);
