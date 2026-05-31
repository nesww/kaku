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
#include "fonts/default8x16.h"
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

    vesa_init();

    uint32_t *vesa_fb = vesa_get_fb();

    SERIAL_KERNEL("VESA address after init: %x\n", vesa_fb);

    for (int x = 0; x < 1024; x++) {
        vesa_fb[x] = 0x00FF0000;
    }

    vesa_putchar('o', 200, 200, 0x00FFFFFF, 0x0);

SERIAL_INFO("pitch: %x\n", *(uint16_t*)0x7E10);

    uint8_t *g = default8x16_psf + 6 + ('A' * 16);
    SERIAL_INFO("glyph A: %x %x %x %x\n", g[0], g[1], g[2], g[3]);

    SERIAL_INFO("bytes 0-7: %x %x %x %x %x %x %x %x\n",
        default8x16_psf[0], default8x16_psf[1],
        default8x16_psf[2], default8x16_psf[3],
        default8x16_psf[4], default8x16_psf[5],
        default8x16_psf[6], default8x16_psf[7]);
    SERIAL_INFO("byte 1044: %x\n", default8x16_psf[1044]);

    for (int i = 1040; i < 1060; i++) {
        SERIAL_INFO("psf[%x]: %x\n", i, default8x16_psf[i]);
    }
    while(1);
}
