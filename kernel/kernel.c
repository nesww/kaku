/*hardware related includes */
#include "fs/ext2/ext2.h"
#include "fs/ext2/inode/inode.h"
#include "hw/idt/idt.h"
#include "hw/pic/pic.h"
#include "hw/pit/pit.h"
#include "hw/serial/serial.h"
#include "hw/vga/vga.h"

/*kernel related includes */
#include "alloc/alloc.h"
#include "alloc/alloc.h"
#include "frame/frame.h"
#include "paging/paging.h"
#include "proc/sched.h"

/* libs includes */
// #include <stdint.h>

#define SOS_VER_MAJOR 0
#define SOS_VER_MINOR 0
#define SOS_VER_PATCH 1

static void __hw_init(void) {
    serial_init();
    vga_clear();
    vga_enable_cursor();
    pic_init();
    pit_init();
    idt_init();
}

static void __fs_init(void) {
    fs_ext2_read_superblock();
    fs_ext2_read_bgdt();
}

static void __kernel_init(void) {
    kheap_init();
    fa_init();
    paging_kernel_init();
    __fs_init();
    scheduler_init();
    INTERRUPTS_ENABLE();

    //replace for journal when available
    vga_printf("sOS - v%d.%d.%d\n", SOS_VER_MAJOR, SOS_VER_MINOR, SOS_VER_PATCH);
    vga_printf("kernel started successfully!\n");
}

void kernel_main(void) {
    __hw_init();
    __kernel_init();

    const fs_ext2_superblock *sb = fs_ext2_get_superblock();
    serial_printf("EXT2: superblock has_extended: %x\n", sb->sb_has_extended);

    const fs_ext2_descriptor *bgdt = fs_ext2_get_bgdt();
    serial_printf("dsc_unallocated_inodes_count_in_group: %x\n", bgdt[0].dsc_unallocated_inodes_count_in_group);

    fs_ext2_inode root = fs_ext2_read_inode(2);
    serial_printf("root inode size_low_bits: %x\n", root.inode_size_low_bits);

    while(1);
}
