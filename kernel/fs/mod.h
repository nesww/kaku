#include <stdint.h>

/*===========================================================================
 * fs — filesystem public API
 *
 * selective includes: define FS_<SUBMOD>_IPL before including
 * this header to pull in a specific subsystem.
 *
 *   #define FS_VFS_IPL
 *   #define FS_FD_IPL
 *   #define FS_EXT2_IPL
 *   #include <fs/mod.h>
 *
 * or define FS_ALL_IPL to include everything.
 *===========================================================================*/

#ifdef FS_ALL_IPL
#  define FS_VFS_IPL
#  define FS_FD_IPL
#  define FS_EXT2_IPL
#endif

/*===========================================================================
 * fs/vfs — Virtual File System
 *===========================================================================*/
#if defined(FS_VFS_IPL) && !defined(FS_VFS_IPL_GUARD)
#  define FS_VFS_IPL_GUARD
#  include "src/vfs.h"
#  include "src/ext2.h"
#  include "src/ext2_vfs.h"
#  include "src/inode.h"

typedef int fs_fd;
#define FS_FD_INVALID (-1)

#define FS_FLAGS_RO 0x1
#define FS_FLAGS_WO 0x2
#define FS_FLAGS_RW 0x4

static inline void fs_init(void) {
    fs_ext2_read_superblock();
    fs_ext2_read_bgdt();
    vfs_register_driver(vfs_get_ext2_driver());
}

static inline vfs_node *fs_open(const char *path)                       { return vfs_open(path); }
static inline int       fs_read(vfs_node *node, uint8_t *buf, uint32_t size) { return vfs_read(node, buf, size); }
static inline int       fs_write(vfs_node *node, uint8_t *buf, uint32_t size) { return vfs_write(node, buf, size); }
static inline void      fs_close(vfs_node *node)                        { vfs_close(node); }
#endif

/*===========================================================================
 * fs/fd — File descriptors
 *===========================================================================*/
#if defined(FS_FD_IPL) && !defined(FS_FD_IPL_GUARD)
#  define FS_FD_IPL_GUARD
#  ifndef FS_VFS_IPL
#    include "src/vfs.h"
#  endif
#  include "src/fd.h"

static inline void            fs_fd_init(void)                          { fd_init_table(); }
static inline uint32_t        fs_fd_new(vfs_node *node, uint8_t flags)  { return fd_new(node, flags); }
static inline fd_table_entry *fs_fd_get(uint32_t fd)                    { return fd_get(fd); }
static inline void            fs_fd_remove(uint32_t fd)                 { fd_remove(fd); }
#endif

/*===========================================================================
 * fs/ext2 — EXT2 filesystem internals
 *===========================================================================*/
#if defined(FS_EXT2_IPL) && !defined(FS_EXT2_IPL_GUARD)
#  define FS_EXT2_IPL_GUARD
#  include "src/ext2.h"
#  include "src/inode.h"
#  include "src/ext2_entry.h"
#endif
