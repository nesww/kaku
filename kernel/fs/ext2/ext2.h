#ifndef FS_EXT2_H
#define FS_EXT2_H

#include "inode/inode.h"
#include <stdint.h>

#define FS_MBR_LBA 0
#define FS_EXT2_SIGNATURE 0xef53

typedef struct {
    uint32_t sb_inodes_count;
    uint32_t sb_blocks_count;
    uint32_t sb_nb_block_for_su;
    uint32_t sb_unallocated_blocks_count;
    uint32_t sb_unallocated_inodes_count;
    uint32_t sb_block_nb_of_sb;
    uint32_t sb_log_block_size;
    uint32_t sb_log_fragment_size;
    uint32_t sb_nb_blocks_in_group;
    uint32_t sb_nb_fragments_in_group;
    uint32_t sb_nb_inodes_in_group;
    uint32_t sb_last_mount_time;
    uint32_t sb_last_written_time;
    uint16_t sb_nb_times_mounted_since_fsck;
    uint16_t sb_nb_mounts_allowed_before_fsck;
    uint16_t sb_ext2_signature;
    uint16_t sb_fs_state;
    uint16_t sb_error_behavior;
    uint16_t sb_version_minor;
    uint32_t sb_posix_time_last_fsck;
    uint32_t sb_interval_between_forced_fsck;
    uint32_t sb_opsys_id;
    uint32_t sb_version_major;
    uint16_t sb_userid_for_reserved_block_use;
    uint16_t sb_groupid_for_reserved_block_use;
} fs_ext2_superblock_base;

typedef struct {
    uint32_t sbx_first_non_reserved_inode;
    uint16_t sbx_inode_size;
    uint16_t sbx_block_group_of_sbx;
    uint32_t sbx_optional_features_flags;
    uint32_t sbx_required_features_flags;
    uint32_t sbx_readonly_features_flags;
    uint8_t  sbx_fs_id[16];
    uint8_t  sbx_volume_name[16]; //null terminated
    uint8_t  sbx_last_mount_path[64]; //null terminated
    uint32_t sbx_compression_alg; //see required features
    uint8_t  sbx_nb_of_blocks_to_prealloc_for_files;
    uint8_t  sbx_nb_of_blocks_to_prealloc_for_directories;
    uint16_t __sbx__unused;
    uint8_t  sbx_journal_id[16];
    uint32_t sbx_journal_inode;
    uint32_t sbx_journal_device;
    uint32_t sbx_head_of_orphan_inode_list;
} fs_ext2_superblock_extended;

typedef struct {
    uint8_t sb_has_extended;
    fs_ext2_superblock_base base;
    fs_ext2_superblock_extended extended;
} fs_ext2_superblock;

typedef struct {
    uint32_t dsc_block_address_of_block_usage_bitmap;
    uint32_t dsc_block_address_of_inode_usage_bitmap;
    uint32_t dsc_starting_block_of_inode_table;
    uint16_t dsc_unallocated_blocks_count_in_group;
    uint16_t dsc_unallocated_inodes_count_in_group;
    uint16_t dsc_directories_count_in_group;
    uint8_t __dsc_padding[14];
} fs_ext2_descriptor;

void fs_ext2_read_superblock(void);
const fs_ext2_superblock *fs_ext2_get_superblock(void);

void fs_ext2_read_bgdt(void);
const fs_ext2_descriptor *fs_ext2_get_bgdt(void);
fs_ext2_inode fs_ext2_read_inode(uint32_t id);
#endif //FS_EXT2_H
