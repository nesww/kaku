/*hardware related includes */
#include "fs/ext2/ext2.h"
#include "hw/idt/idt.h"
#include "hw/pic/pic.h"
#include "hw/pit/pit.h"
#include "hw/serial/serial.h"
#include "hw/vesa/vesa.h"
#include "panic/panic.h"
#include "tty/tty.h"

/*kernel related includes */
#include "alloc/alloc.h"
#include "alloc/alloc.h"
#include "frame/frame.h"
#include "paging/paging.h"
#include "proc/sched.h"

/* libs includes */
// #include <stdint.h>

#define KAKU_VER_MAJOR 0
#define KAKU_VER_MINOR 0
#define KAKU_VER_PATCH 3

#define KAKU_ASCII \
"         @@       @@@        \t    .-. .-')    ('-.    .-. .-')              \n"       \
"         @          @        \t    \\  ( OO )  ( OO ).-.\\  ( OO )             \n"     \
"         @          -@@@@@#  \t    ,--. ,--.  / . --. /,--. ,--. ,--. ,--.   \n"       \
"        .@@@@ @@-*@          \t    |  .'   /  | \\-.  \\ |  .'   / |  | |  |   \n"     \
"        @@      .@   @@      \t    |      /,.-'-'  |  ||      /, |  | | .-') \n"       \
"       @%@@    @.   @%       \t      |     ' _)\\| |_.'  ||     ' _)|  |_|( OO )\n"      \
"      @  @ -@ @*  @@  @@     \t    |  .   \\   |  .-.  ||  .   \\  |  | | `-' / \n"    \
"    @.   @      @@   @@.     \t    |  |\\   \\  |  | |  ||  |\\   \\('  '-'(_.-' \n"   \
"         @    @@    @@       \t    `--' '--'  `--' `--'`--' '--'  `-----'     \n"      \
"        @@  @.    @@  @@@    \t                                           \n"          \
"        @%     .@.      @@   \t                                           \n"          \

static void __hw_init(void) {
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
    vesa_init();
    __fs_init();
    scheduler_init();
    INTERRUPTS_ENABLE();
}

static void __kernel_print_info() {
    serial_printf("\f\n===========================================================\n");
    serial_printf("   kaku - v%d.%d.%d\n", KAKU_VER_MAJOR, KAKU_VER_MINOR, KAKU_VER_PATCH);
    serial_printf(KAKU_ASCII);
    serial_printf("===========================================================\n\n");
    tty_printf("\n===========================================================\n");
    tty_printf("   kaku - v%d.%d.%d\n", KAKU_VER_MAJOR, KAKU_VER_MINOR, KAKU_VER_PATCH);
    tty_printf(KAKU_ASCII);
    tty_printf("===========================================================\n\n");
    SERIAL_KERNEL("everything initialized, kernel running\n\n");
    TTY_KERNEL("everything initialized, kernel running\n\n");
}

void kernel_main(void) {
    serial_init();
    __hw_init();
    __kernel_init();
    tty_init();
     __kernel_print_info();
    TTY_WARN("No entry process was started, since none was given...\n");
    TTY_INFO("idling...\n");

    while(1);
}
