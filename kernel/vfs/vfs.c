#include <tty/tty.h>

#include "vfs.h"

static vfs_driver *__vfs_driver = 0;

void vfs_register_driver(vfs_driver *driver) {
    __vfs_driver = driver;
}

vfs_node *vfs_open(const char *path) {
    TTY_INFO("open: %s\n", path);
    if (!__vfs_driver)
        return 0;

    return __vfs_driver->open(path);
}

dynarray *vfs_list(vfs_node *node) {
    if (!__vfs_driver)
        return 0;

    return __vfs_driver->list(node);
}

int vfs_read(vfs_node *node, uint8_t *buf, uint32_t size) {
    if (!__vfs_driver)
        return -1;

    return __vfs_driver->read(node, buf, size);
}

int vfs_write(vfs_node *node, uint8_t *buf, uint32_t size) {
    if (!__vfs_driver)
        return -1;

    return __vfs_driver->write(node, buf, size);
}

void vfs_close(vfs_node *node) {
    if (!__vfs_driver)
        return;

    __vfs_driver->close(node);
}
