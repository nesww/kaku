#include "fs/ext2/ext2.h"
#include "fs/ext2/inode/inode.h"
#include "lib/dynarray.h"
#include "lib/string.h"
#include "vfs/vfs.h"

static vfs_node *__ext2_open(const char *path) {
    fs_ext2_inode inode = fs_ext2_resolve_path(path);
    if (inode.inode_type_and_permissions == 0) {
        return 0;
    }
    vfs_node *node = kmalloc(sizeof(vfs_node));
    node->size = inode.inode_size_low_bits;
    node->is_dir = fs_ext2_is_inode_dir(&inode);
    kstrcpy(node->name, path);
    fs_ext2_inode *inode_copy = kmalloc(sizeof(fs_ext2_inode));
    *inode_copy = inode;
    node->fs_specific = inode_copy;
    return node;
}

static dynarray *__ext2_list(vfs_node *node) {
    return fs_ext2_list_dir(node->fs_specific);
}

static int __ext2_read(vfs_node *node, uint8_t *buf, uint32_t size) {
    return fs_ext2_read_file_contents(node->fs_specific, buf, size);
}

static int __ext2_write(vfs_node *node, uint8_t *buf, uint32_t size) {
    return fs_ext2_write_file(node->fs_specific, buf, size);
}

static void __ext2_close(vfs_node *node) {
    kfree(node->fs_specific);
    kfree(node);
}

vfs_driver *vfs_get_ext2_driver(void) {
    static vfs_driver driver = {
        .open  = __ext2_open,
        .list  = __ext2_list,
        .read  = __ext2_read,
        .write = __ext2_write,
        .close = __ext2_close,
    };
    return &driver;
}
