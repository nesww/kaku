#pragma once

#include <stdint.h>

typedef struct {
    uint32_t entry_inode;
    uint16_t entry_size;
    uint8_t  entry_name_len;
    uint8_t  entry_type;
    uint8_t  entry_name[256];
} fs_ext2_entry;
