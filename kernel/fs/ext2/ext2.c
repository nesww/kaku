#include "ext2.h"
#include "disk/ata/ata.h"
#include "fs/ext2/inode/inode.h"
#include "lib/core.h"
#include "lib/math.h"
#include "lib/stdmem.h"
#include "panic/panic.h"

static fs_ext2_superblock fs_ext2_sb = {0};
static fs_ext2_descriptor *fs_ext2_bgdt;
static uint8_t  __fs_ext2_sb_read   = FALSE;
static uint8_t  __fs_ext2_bgdt_read = FALSE;
static uint32_t __fs_ext2_nb_block_groups;
static uint32_t __fs_ext2_block_size;
static uint32_t __fs_ext2_lba_root;

void fs_ext2_read_superblock(void) {
    uint16_t *buf_mbr = kmalloc(512);

    ata_read(FS_MBR_LBA, 1, buf_mbr);

    //get the LBA of the ext2 superblock from MBR at offset 0x1be
    __fs_ext2_lba_root = *(uint32_t*)((uint8_t*)buf_mbr + 0x1be + 8);
    uint32_t lba_ext2_sb = __fs_ext2_lba_root + 2;
    kfree(buf_mbr);

    uint16_t *buf_lba_ext2 = kmalloc(KB(1));
    ata_read(lba_ext2_sb, 2, buf_lba_ext2);

    fs_ext2_superblock sb;
    sb.base = *(fs_ext2_superblock_base*)buf_lba_ext2;

    sb.sb_has_extended = sb.base.sb_ext2_signature == FS_EXT2_SIGNATURE;

    if (!sb.sb_has_extended) {
        sb.extended.sbx_inode_size = 128;
        sb.extended.sbx_first_non_reserved_inode = 11;
        goto read_superblock_end;
    }
    sb.extended = *(fs_ext2_superblock_extended*)((uint8_t*)buf_lba_ext2 + 84);
    read_superblock_end:
        kfree(buf_lba_ext2);
        fs_ext2_sb = sb;
        __fs_ext2_sb_read = TRUE;
        __fs_ext2_block_size = 1024 << sb.base.sb_log_block_size;
        __fs_ext2_nb_block_groups = CEIL_DIV(fs_ext2_sb.base.sb_blocks_count, fs_ext2_sb.base.sb_nb_blocks_in_group);
        return;
}

const fs_ext2_superblock *fs_ext2_get_superblock(void) {
    return &fs_ext2_sb;
}

void fs_ext2_read_bgdt(void) {
    if (!__fs_ext2_sb_read)
        kernel_panic("FS_EXT2_NO_SB: tried to read BGDT on disk when superblock was not read");

    uint32_t bgdt_size = __fs_ext2_nb_block_groups * sizeof(fs_ext2_descriptor);
    uint16_t *buf_bgdt = kmalloc(bgdt_size);

    ata_read(__fs_ext2_lba_root + 4, CEIL_DIV(bgdt_size, ATA_SECTOR_SIZE), buf_bgdt);

    fs_ext2_bgdt = kmalloc(bgdt_size);
    kmemcpy(fs_ext2_bgdt, buf_bgdt, bgdt_size);

    __fs_ext2_bgdt_read = TRUE;
    kfree(buf_bgdt);
}

const fs_ext2_descriptor *fs_ext2_get_bgdt(void) {
    return fs_ext2_bgdt;
}

void __fs_ext2_allread(const char *func_name) {
    if (!__fs_ext2_sb_read)
        kernel_panicf("%s: could not do operation, superblock was not read\n", func_name);
    if (!__fs_ext2_bgdt_read)
        kernel_panicf("%s: could not do operation, BGDT was not read\n", func_name);
}

fs_ext2_inode fs_ext2_read_inode(uint32_t inode_id) {
    __fs_ext2_allread(__func__);

    uint32_t blk_group = (inode_id - 1) / fs_ext2_sb.base.sb_nb_inodes_in_group;
    uint32_t inode_index = (inode_id - 1) % fs_ext2_sb.base.sb_nb_inodes_in_group;
    uint32_t blk =
        fs_ext2_bgdt[blk_group].dsc_starting_block_of_inode_table
        + (inode_index * fs_ext2_sb.extended.sbx_inode_size) / __fs_ext2_block_size;

    uint32_t inode_lba = __fs_ext2_lba_root + (blk * __fs_ext2_block_size / ATA_SECTOR_SIZE);
    uint16_t *buf_inode = kmalloc(sizeof(fs_ext2_inode));

    ata_read(inode_lba, CEIL_DIV(sizeof(fs_ext2_inode), ATA_SECTOR_SIZE), buf_inode);

    uint32_t offset_in_block = (inode_index * fs_ext2_sb.extended.sbx_inode_size) % __fs_ext2_block_size;
    fs_ext2_inode inode = *(fs_ext2_inode*)(buf_inode + offset_in_block);

    kfree(buf_inode);
    return inode;
}
