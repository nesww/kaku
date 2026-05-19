/*hardware related includes */
#include "disk/ata/ata.h"
#include "hw/idt/idt.h"
#include "hw/pic/pic.h"
#include "hw/pit/pit.h"
#include "hw/serial/serial.h"
#include "hw/vga/vga.h"

/*kernel related includes */
#include "alloc/alloc.h"
#include "alloc/alloc.h"
#include "frame/frame.h"
#include "lib/core.h"
#include "lib/stdmem.h"
#include "paging/paging.h"
#include "proc/sched.h"

/* libs includes */
#include <stdint.h>

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

static void __kernel_init(void) {
    kheap_init();
    fa_init();
    paging_kernel_init();
    scheduler_init();
    INTERRUPTS_ENABLE();

    //replace for journal when available
    vga_printf("sOS - v%d.%d.%d\n", SOS_VER_MAJOR, SOS_VER_MINOR, SOS_VER_PATCH);
    vga_printf("kernel started successfully!\n");
}

void kernel_main(void) {
    __hw_init();
    __kernel_init();

    uint16_t *buf = kmalloc(KB(1));

    serial_printf("Bootloader read from disk :) :\n");
    ata_read(0, 1, buf);
    for (uint32_t i = 0; i < 256; ++i) {
        serial_printf(" %x%s", buf[i], ((i+1)%5 == 0 ? "\n": ""));
    }
    while(1);
}
