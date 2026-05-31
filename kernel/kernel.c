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
#include "lib/string.h"
#include "paging/paging.h"
#include "proc/sched.h"
#include <stdint.h>

/* libs includes */
// #include <stdint.h>

#define SOS_VER_MAJOR 0
#define SOS_VER_MINOR 0
#define SOS_VER_PATCH 3

static void __hw_init(void) {
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

static void __kernel_print_info() {
    serial_printf("\f\n===========\n");
    serial_printf("sOS - v%d.%d.%d\n", SOS_VER_MAJOR, SOS_VER_MINOR, SOS_VER_PATCH);
    serial_printf("===========\n\n");
}

void kernel_main(void) {
    serial_init();
    __kernel_print_info();
    __hw_init();
    __kernel_init();

    SERIAL_KERNEL("everything initialized, kernel running\n\n");

    fs_ext2_inode root = fs_ext2_read_inode(2);
    SERIAL_KERNEL("root direct_ptr0: %x\n", root.inode_direct_blk_ptr0);



    fs_ext2_inode toto = fs_ext2_resolve_path("/toto.txt");
    SERIAL_KERNEL("is toto inode a file ? %x\n", fs_ext2_is_inode_file(&toto));

    fs_ext2_inode grub_dir = fs_ext2_resolve_path("/etc/grub");
    SERIAL_KERNEL("is grub_dir inode a dir ? %x\n", fs_ext2_is_inode_dir(&grub_dir));

    fs_ext2_inode config = fs_ext2_resolve_path("/config");



    uint8_t *buf_config = kmalloc(KB(10));
    fs_ext2_read_file_contents(&config, buf_config, KB(10));
    SERIAL_KERNEL("after reading file contents\n");
    SERIAL_KERNEL("low_size_bits: %x\n", config.inode_size_low_bits);
    SERIAL_INFO("config.inode_direct_blk_ptr0: %x\n", config.inode_direct_blk_ptr0);
    buf_config[config.inode_size_low_bits] = '\0';

    SERIAL_INFO("buf[0..3]: %x %x %x %x\n", buf_config[0], buf_config[1], buf_config[2], buf_config[3]);

    SERIAL_KERNEL("following, raw buffer read from file via fs:\n");
    serial_printf("%s", buf_config);

    uint8_t *new_content = (uint8_t*)"hello from kernel!\n";
    fs_ext2_create_file("/newfile.txt", new_content, 19);
    fs_ext2_inode newfile = fs_ext2_resolve_path("/newfile.txt");

    uint8_t *buf_new = kmalloc(256);
    fs_ext2_read_file_contents(&newfile, buf_new, 256);
    buf_new[newfile.inode_size_low_bits] = '\0';
    serial_printf("%s\n", buf_new);


    fs_ext2_inode config2 = fs_ext2_resolve_path("/config");

    uint8_t *new_content_config = (uint8_t*)"modified by kernel fs!\n";
    fs_ext2_write_file(&config2, new_content_config, kstrlen((const char*)new_content_config));
    uint8_t *buf_config_modified = kmalloc(256);
    fs_ext2_read_file_contents(&config2, buf_config_modified, 256);
    buf_config_modified[config2.inode_size_low_bits] = '\0';
    serial_printf("%s\n", buf_config_modified);
    while(1);
}
