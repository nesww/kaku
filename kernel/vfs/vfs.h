#pragma once

#include <stdint.h>
#include <lib/dynarray.h>

typedef struct {
    char name[256];
    uint32_t size;
    uint8_t is_dir;
    void *fs_specific;
} vfs_node;

typedef struct {
    vfs_node* (*open)(const char *path);
    dynarray* (*list)(vfs_node* node);
    int       (*read)(vfs_node *node, uint8_t *buf, uint32_t size);
    int       (*write)(vfs_node *node, uint8_t *buf, uint32_t size);
    void      (*close)(vfs_node *node);
} vfs_driver;

void vfs_register_driver(vfs_driver *driver);

vfs_node *vfs_open(const char *path);
dynarray *vfs_list(vfs_node *node);
int vfs_read(vfs_node *node, uint8_t *buf, uint32_t size);
int vfs_write(vfs_node *node, uint8_t *buf, uint32_t size);
void vfs_close(vfs_node *node);
