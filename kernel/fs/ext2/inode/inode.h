#ifndef FS_EXT2_INODE_H
#define FS_EXT2_INODE_H

#include <stdint.h>

typedef struct {
    uint16_t inode_type_and_permissions;
    uint16_t inode_user_id;

    uint32_t inode_size_low_bits;
    uint32_t inode_last_access_time;       //in POSIX time
    uint32_t inode_creation_time;          //in POSIX time
    uint32_t inode_last_modification_time; //in POSIX time
    uint32_t inode_deletion_time;          //in POSIX time

    uint16_t inode_groupid;
    uint16_t inode_hard_links_count;

    uint32_t inode_used_disk_sectors_count;
    uint32_t inode_flags;
    uint32_t __inode_unused_os_specific;

    uint32_t inode_direct_blk_ptr1;
    uint32_t inode_direct_blk_ptr2;
    uint32_t inode_direct_blk_ptr3;
    uint32_t inode_direct_blk_ptr4;
    uint32_t inode_direct_blk_ptr5;
    uint32_t inode_direct_blk_ptr6;
    uint32_t inode_direct_blk_ptr7;
    uint32_t inode_direct_blk_ptr8;
    uint32_t inode_direct_blk_ptr9;
    uint32_t inode_direct_blk_ptr10;
    uint32_t inode_direct_blk_ptr11;

    uint32_t inode_singly_indirect_blk_ptr;
    uint32_t inode_doubly_indirect_blk_ptr;
    uint32_t inode_triply_indirect_blk_ptr;

    uint32_t inode_generation_number;
    uint32_t inode_extended_attr_blk;
    uint32_t inode_size_high_bits_file__dir_acl;
    uint32_t inode_block_address_of_fragment;
    uint8_t  __inode_unused_os_specific2[12];
} fs_ext2_inode;

#endif
