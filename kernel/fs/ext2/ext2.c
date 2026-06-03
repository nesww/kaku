#include "ext2.h"
#include "disk/ata/ata.h"
#include "fs/ext2/entry/entry.h"
#include "fs/ext2/inode/inode.h"
#include "hw/serial/serial.h"
#include "lib/core.h"
#include "lib/dynarray.h"
#include "lib/math.h"
#include "lib/stdmem.h"
#include "lib/string.h"
#include "panic/panic.h"
#include <stdint.h>

typedef uint32_t fs_ext2_blk;
typedef uint32_t fs_ext2_lba;
typedef uint16_t *fs_ext2_ata_buf;

static fs_ext2_superblock fs_ext2_sb = {0};
static fs_ext2_descriptor *fs_ext2_bgdt;
static uint8_t  __fs_ext2_sb_read   = FALSE;
static uint8_t  __fs_ext2_bgdt_read = FALSE;
static uint32_t __fs_ext2_nb_block_groups;
static uint32_t __fs_ext2_block_size;
static fs_ext2_lba __fs_ext2_lba_root;

#define FS_EXT2_BLK_TO_LBA(blk) (__fs_ext2_lba_root + ((blk) * __fs_ext2_block_size / ATA_SECTOR_SIZE))
#define FS_EXT2_INODEID_TO_BLK_GRP(inode_id) ((inode_id - 1) / fs_ext2_sb.base.sb_nb_inodes_in_group)
#define FS_EXT2_INODEID_TO_INODE_INDEX(inode_id) ((inode_id - 1) % fs_ext2_sb.base.sb_nb_inodes_in_group)

void fs_ext2_read_superblock(void) {
    fs_ext2_ata_buf buf_mbr = kmalloc(512);

    ata_read(FS_MBR_LBA, 1, buf_mbr);

    //get the LBA of the ext2 superblock from MBR at offset 0x1be
    __fs_ext2_lba_root = *(uint32_t*)((uint8_t*)buf_mbr + 0x1be + 8);
    fs_ext2_lba lba_ext2_sb = __fs_ext2_lba_root + 2;
    kfree(buf_mbr);

    fs_ext2_ata_buf buf_lba_ext2 = kmalloc(KB(1));
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
    fs_ext2_ata_buf buf_bgdt = kmalloc(bgdt_size);

    ata_read(__fs_ext2_lba_root + 4, CEIL_DIV(bgdt_size, ATA_SECTOR_SIZE), buf_bgdt);

    fs_ext2_bgdt = kmalloc(bgdt_size);
    kmemcpy(fs_ext2_bgdt, buf_bgdt, bgdt_size);

    __fs_ext2_bgdt_read = TRUE;
    kfree(buf_bgdt);
}

const fs_ext2_descriptor *fs_ext2_get_bgdt(void) {
    return fs_ext2_bgdt;
}

static void __fs_ext2_allread(const char *func_name) {
    if (!__fs_ext2_sb_read)
        kernel_panicf("%s: could not do operation, superblock was not read\n", func_name);
    if (!__fs_ext2_bgdt_read)
        kernel_panicf("%s: could not do operation, BGDT was not read\n", func_name);
}

fs_ext2_inode fs_ext2_read_inode(uint32_t inode_id) {
    __fs_ext2_allread(__func__);

    uint32_t blk_group = FS_EXT2_INODEID_TO_BLK_GRP(inode_id);
    uint32_t inode_index = FS_EXT2_INODEID_TO_INODE_INDEX(inode_id);
    fs_ext2_blk blk =
        fs_ext2_bgdt[blk_group].dsc_starting_block_of_inode_table
        + (inode_index * fs_ext2_sb.extended.sbx_inode_size) / __fs_ext2_block_size;

    fs_ext2_lba inode_lba = FS_EXT2_BLK_TO_LBA(blk);
    fs_ext2_ata_buf buf_inode = kmalloc(__fs_ext2_block_size);

    ata_read(inode_lba, __fs_ext2_block_size/ATA_SECTOR_SIZE, buf_inode);

    uint32_t offset_in_block = (inode_index * fs_ext2_sb.extended.sbx_inode_size) % __fs_ext2_block_size;

    uint8_t *raw = (uint8_t*)buf_inode + offset_in_block;

    fs_ext2_inode inode = *(fs_ext2_inode*)((uint8_t *)buf_inode + offset_in_block);
    inode.inode_id = inode_id;

    kfree(buf_inode);
    return inode;
}

void fs_ext2_write_inode(fs_ext2_inode *inode) {
    __fs_ext2_allread(__func__);

    uint32_t blk_group = FS_EXT2_INODEID_TO_BLK_GRP(inode->inode_id);
    uint32_t inode_index = FS_EXT2_INODEID_TO_INODE_INDEX(inode->inode_id);
    fs_ext2_blk blk =
        fs_ext2_bgdt[blk_group].dsc_starting_block_of_inode_table
        + (inode_index * fs_ext2_sb.extended.sbx_inode_size) / __fs_ext2_block_size;

    fs_ext2_lba inode_lba = FS_EXT2_BLK_TO_LBA(blk);
    fs_ext2_ata_buf buf_inode = kmalloc(__fs_ext2_block_size);
    uint32_t offset_in_block = (inode_index * fs_ext2_sb.extended.sbx_inode_size) % __fs_ext2_block_size;

    ata_read(inode_lba, __fs_ext2_block_size/ATA_SECTOR_SIZE, buf_inode);

    kmemcpy((uint8_t*)buf_inode + offset_in_block, inode, sizeof(fs_ext2_inode) - sizeof(uint32_t));
    ata_write(inode_lba, __fs_ext2_block_size/ATA_SECTOR_SIZE, buf_inode);

    kfree(buf_inode);
}

dynarray *fs_ext2_list_dir(fs_ext2_inode *inode) {
    if ((inode->inode_type_and_permissions & 0xf000) != FS_EXT2_INODE_FTYPE_DIR) {
        SERIAL_ERROR("given inode was not a directory: %x\n", __func__, inode->inode_type_and_permissions);
        return 0;
    }

    dynarray *entries = dynarray_new(32);

    uint32_t nb_blocks = CEIL_DIV(inode->inode_size_low_bits, __fs_ext2_block_size);
    fs_ext2_blk *direct_ptrs = &inode->inode_direct_blk_ptr0;

    for (uint32_t i = 0; i < nb_blocks; ++i) {
        fs_ext2_blk blk = direct_ptrs[i];
        fs_ext2_lba lba = FS_EXT2_BLK_TO_LBA(blk);
        fs_ext2_ata_buf buf = kmalloc(__fs_ext2_block_size);
        ata_read(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf);

        uint32_t offset = 0;
        while (offset < __fs_ext2_block_size) {
            fs_ext2_entry *entry_from_buf = (fs_ext2_entry*)((uint8_t*)buf + offset);
            uint8_t *entry_name = (uint8_t*)entry_from_buf + 8;

            fs_ext2_entry *entry = kmalloc(sizeof(fs_ext2_entry));

            kmemcpy(entry, entry_from_buf, 8);
            kmemcpy(entry->entry_name, entry_name, entry_from_buf->entry_name_len);
            entry->entry_name[entry_from_buf->entry_name_len] = '\0';

            dynarray_add(entries, entry);

            offset += entry_from_buf->entry_size;
        }
        kfree(buf);
    }
    return entries;
}

static void __fs_ext2_free_entries_of_dir_da(void *entry) {
    kfree(entry);
}

uint32_t __fs_ext2_find_in_dir(fs_ext2_inode *dir, const char *name) {
    uint32_t inode_id = 0;
    dynarray *entries_of_dir = fs_ext2_list_dir(dir);

    for (uint32_t i = 0; i < entries_of_dir->count; ++i) {
        fs_ext2_entry *entry = (fs_ext2_entry*)dynarray_get(entries_of_dir, i);
        if (kstrcmp(name, (const char*)entry->entry_name) == 0) {
            inode_id = entry->entry_inode;
            break;
        }
    }
    dynarray_free_deep(entries_of_dir, __fs_ext2_free_entries_of_dir_da);
    return inode_id;
}

static void __fs_ext2_free_names(void *name) {
    kfree(name);
}

fs_ext2_inode fs_ext2_resolve_path(const char* path) {
    fs_ext2_inode fs_root = fs_ext2_read_inode(2);
    if (kstrlen(path) == 1 && path[0] == '/') return fs_root;
    dynarray *names = kstrsplit(path, '/');

    uint32_t start = path[0] == '/' ? 1 :  0;
    char *name = dynarray_get(names, start);

    uint32_t next_inode_id = __fs_ext2_find_in_dir(&fs_root, name);
    if (next_inode_id == 0) {
        goto resolve_path_err;
    }
    fs_ext2_inode next_inode = fs_ext2_read_inode(next_inode_id);

    uint32_t i = 1 + start;
    while (i < names->count) {
        name = dynarray_get(names, i);
        next_inode_id = __fs_ext2_find_in_dir(&next_inode, name);
        if (next_inode_id == 0) {
            goto resolve_path_err;
        }
        next_inode = fs_ext2_read_inode(next_inode_id);
        ++i;
    }

    dynarray_free_deep(names, __fs_ext2_free_names);
    return next_inode;
    resolve_path_err:
        SERIAL_ERROR("not found: '%s' in path: '%s', does not exist\n", name, path);
        dynarray_free_deep(names, __fs_ext2_free_names);
        return (fs_ext2_inode){0};
}

int fs_ext2_read_file_contents(fs_ext2_inode *inode, uint8_t *buf, uint32_t size) {
    if (!fs_ext2_is_inode_file(inode)) {
        SERIAL_ERROR("given inode is not a file, cannot read bytes\n");
        return -1;
    }

    uint32_t i = 0;
    uint32_t written_bytes = 0;
    uint32_t nb_blocks = CEIL_DIV(inode->inode_size_low_bits, __fs_ext2_block_size);
    fs_ext2_blk *direct_ptrs = &inode->inode_direct_blk_ptr0;
    while (i < nb_blocks && written_bytes < size) {
        uint32_t nb_of_bytes_to_copy = KMIN(size - written_bytes, __fs_ext2_block_size);

        fs_ext2_blk blk = direct_ptrs[i];
        fs_ext2_lba lba = FS_EXT2_BLK_TO_LBA(blk);

        fs_ext2_ata_buf buf_blk = kmalloc(__fs_ext2_block_size);
        ata_read(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_blk);

        kmemcpy(buf + written_bytes, buf_blk, nb_of_bytes_to_copy);
        kfree(buf_blk);
        written_bytes += nb_of_bytes_to_copy;
        ++i;
    }
    return written_bytes;
}

void __fs_ext2_write_bgdt_to_disk(void) {
    ata_write(__fs_ext2_lba_root + 4, CEIL_DIV(__fs_ext2_nb_block_groups * sizeof(fs_ext2_descriptor), ATA_SECTOR_SIZE), (uint16_t*)fs_ext2_bgdt);
}

void __fs_ext2_write_sb_base_to_disk(void) {
    uint16_t *buf = kmalloc(__fs_ext2_block_size);
    ata_read(__fs_ext2_lba_root + 2, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf);
    kmemcpy(buf, &fs_ext2_sb.base, sizeof(fs_ext2_superblock_base));
    ata_write(__fs_ext2_lba_root + 2, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf);
    kfree(buf);
}

uint32_t fs_ext2_alloc_block(uint32_t preferred_group) {
    uint32_t blk_nb = 0;
    for (uint32_t blk = 0; blk < __fs_ext2_nb_block_groups; ++blk) {
        uint32_t grp = (preferred_group + blk) % __fs_ext2_nb_block_groups;
        fs_ext2_descriptor desc = *(fs_ext2_descriptor*)(fs_ext2_bgdt + grp);

        if (desc.dsc_unallocated_blocks_count_in_group > 0) {
            fs_ext2_ata_buf buf_bitmap = kmalloc(__fs_ext2_block_size);
            fs_ext2_lba lba = FS_EXT2_BLK_TO_LBA(desc.dsc_block_address_of_block_usage_bitmap);

            ata_read(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_bitmap);
            uint8_t *block_bitmap = (uint8_t*)buf_bitmap;

            for (uint32_t byte_index = 0; byte_index < __fs_ext2_block_size; ++byte_index) {
                for (uint8_t bit_index = 0; bit_index < 8; ++bit_index) {
                    if (((block_bitmap[byte_index] >> bit_index) & 0b1) == 0) {
                        blk_nb = grp * fs_ext2_sb.base.sb_nb_blocks_in_group + (byte_index * 8 + bit_index);
                        block_bitmap[byte_index] |= (1 << bit_index);

                        ata_write(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_bitmap);
                        fs_ext2_bgdt[grp].dsc_unallocated_blocks_count_in_group--;
                        fs_ext2_sb.base.sb_unallocated_blocks_count--;

                        __fs_ext2_write_bgdt_to_disk();
                        __fs_ext2_write_sb_base_to_disk();
                        kfree(buf_bitmap);
                        goto alloc_block_end;
                    }
                }
            }
            kfree(buf_bitmap);
        }
    }
    alloc_block_end:
    return blk_nb;
}

uint32_t fs_ext2_alloc_inode(uint32_t preferred_group) {
    uint32_t inode_nb = 0;
    for (uint32_t blk = 0; blk < __fs_ext2_nb_block_groups; ++blk) {
        uint32_t grp = (preferred_group + blk) % __fs_ext2_nb_block_groups;
        fs_ext2_descriptor desc = *(fs_ext2_descriptor*)(fs_ext2_bgdt + grp);

        if (desc.dsc_unallocated_inodes_count_in_group > 0) {
            fs_ext2_ata_buf buf_bitmap = kmalloc(__fs_ext2_block_size);
            fs_ext2_lba lba = FS_EXT2_BLK_TO_LBA(desc.dsc_block_address_of_inode_usage_bitmap);

            ata_read(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_bitmap);
            uint8_t *inode_bitmap = (uint8_t*)buf_bitmap;

            for (uint32_t byte_index = 0; byte_index < fs_ext2_sb.base.sb_nb_inodes_in_group / 8; ++byte_index) {
                for (uint8_t bit_index = 0; bit_index < 8; ++bit_index) {
                    if (((inode_bitmap[byte_index] >> bit_index) & 0b1) == 0) {
                        inode_nb = grp * fs_ext2_sb.base.sb_nb_inodes_in_group + (byte_index * 8 + bit_index) + 1;
                        inode_bitmap[byte_index] |= (1 << bit_index);

                        ata_write(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_bitmap);
                        fs_ext2_bgdt[grp].dsc_unallocated_inodes_count_in_group--;
                        fs_ext2_sb.base.sb_unallocated_inodes_count--;

                        __fs_ext2_write_bgdt_to_disk();
                        __fs_ext2_write_sb_base_to_disk();
                        kfree(buf_bitmap);
                        goto alloc_inode_end;
                    }
                }
            }
            kfree(buf_bitmap);
        }
    }
    alloc_inode_end:
    return inode_nb;
}

int fs_ext2_write_file(fs_ext2_inode *inode, uint8_t *buf, uint32_t size) {
    if (!fs_ext2_is_inode_file(inode)) {
        SERIAL_ERROR("given inode is not a file, will not write bytes to given inode\n");
        return -1;
    }

    if (size != inode->inode_size_low_bits) inode->inode_size_low_bits = size;

    uint32_t i = 0;
    uint32_t written_bytes = 0;
    fs_ext2_blk *direct_ptrs = &inode->inode_direct_blk_ptr0;
    while (written_bytes < size) {
        uint32_t nb_of_bytes_to_copy = KMIN(size - written_bytes, __fs_ext2_block_size);

        fs_ext2_blk blk = direct_ptrs[i];
        if (blk == 0) {
            blk = fs_ext2_alloc_block(0);
            direct_ptrs[i] = blk;
        }
        fs_ext2_lba lba = FS_EXT2_BLK_TO_LBA(blk);

        fs_ext2_ata_buf buf_blk = kmalloc(__fs_ext2_block_size);
        ata_read(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_blk);

        kmemcpy(buf_blk, buf+written_bytes, nb_of_bytes_to_copy);
        ata_write(lba, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_blk);

        kfree(buf_blk);
        written_bytes += nb_of_bytes_to_copy;
        ++i;
    }
    fs_ext2_write_inode(inode);
    return written_bytes;
}

void fs_ext2_create_file(const char *path, uint8_t *buf, uint32_t size) {
    uint32_t inode_id = fs_ext2_alloc_inode(0);

    fs_ext2_inode inode = {0};
    inode.inode_type_and_permissions = 0x81A4;
    inode.inode_size_low_bits = 0;
    inode.inode_hard_links_count = 1;
    inode.inode_id = inode_id;

    fs_ext2_write_file(&inode, buf, size);

    uint8_t *parent_dir;
    uint8_t parent_dir_is_root = FALSE;
    dynarray *path_names = kstrsplit(path, '/');
    uint8_t *filename = dynarray_remove_last(path_names);
    if (path_names->count == 0 || kstrlen((char*)dynarray_get(path_names, 0)) == 0) {
       parent_dir = (uint8_t*)"/";
       parent_dir_is_root = TRUE;
    } else {
        parent_dir = (uint8_t*)kstrjoin(path_names, '/');
    }

    dynarray_free_deep(path_names, __fs_ext2_free_names);

    fs_ext2_inode parent_inode = fs_ext2_resolve_path((const char*)parent_dir);
    if (!fs_ext2_is_inode_dir(&parent_inode)) {
        if (!parent_dir_is_root) kfree(parent_dir);
        return;
    }
    if (!parent_dir_is_root) kfree(parent_dir);

    fs_ext2_blk *parent_direct_ptrs = &parent_inode.inode_direct_blk_ptr0;
    uint32_t parent_nb_blocks       = CEIL_DIV(parent_inode.inode_size_low_bits, __fs_ext2_block_size);
    fs_ext2_blk last_blk            = parent_direct_ptrs[parent_nb_blocks - 1];
    fs_ext2_lba lba_last_blk        = FS_EXT2_BLK_TO_LBA(last_blk);
    fs_ext2_ata_buf buf_last_blk    = kmalloc(__fs_ext2_block_size);
    ata_read(lba_last_blk, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_last_blk);


    uint32_t offset = 0;
    fs_ext2_entry *last_entry;
    uint32_t last_offset = 0;
    while (offset < __fs_ext2_block_size) {
        fs_ext2_entry *parent_entry = (fs_ext2_entry*)((uint8_t*)(buf_last_blk) + offset);
        if (parent_entry->entry_size + offset == __fs_ext2_block_size) {
            last_entry = parent_entry;
            last_offset = offset;
        }
        offset+= parent_entry->entry_size;
    }

    last_entry->entry_size = CEIL_DIV(8 + last_entry->entry_name_len, 4) * 4;

    fs_ext2_entry *new_entry  = (fs_ext2_entry*)((uint8_t*)buf_last_blk + last_offset + last_entry->entry_size);
    new_entry->entry_size     = __fs_ext2_block_size - (last_offset + last_entry->entry_size);
    new_entry->entry_name_len = kstrlen((const char*)filename);
    new_entry->entry_inode    = inode_id;
    new_entry->entry_type     = 1;
    kmemcpy((uint8_t*)new_entry + 8, filename, new_entry->entry_name_len);

    ata_write(lba_last_blk, __fs_ext2_block_size / ATA_SECTOR_SIZE, buf_last_blk);
    kfree(buf_last_blk);
}
