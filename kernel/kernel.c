#define DEV_ALL_IPL
#define HW_ALL_IPL
#define MM_ALL_IPL
#define FS_ALL_IPL
#define PROC_ALL_IPL
#define TTY_ALL_IPL
#define LIB_ALL_IPL

#include <elf/mod.h>
#include <fs/mod.h>
#include <tty/mod.h>
#include <panic/mod.h>
#include <hw/mod.h>
#include <dev/mod.h>
#include <mm/mod.h>
#include <proc/mod.h>
#include <log/mod.h>

#define KAKU_VER_MAJOR 1
#define KAKU_VER_MINOR 0
#define KAKU_VER_PATCH 0

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
    hw_irq_init();
    hw_timer_init(100);
    hw_idt_init();
}

static void __kernel_init(void) {
    mm_init();
    dev_fb_init();
    fs_init();
    tss_init();
    tss_install();
    scheduler_init();
    stdin_init();
    fs_fd_init();
    proc_create_k_idle();
    HW_INT_ENABLE();
}

static void __kernel_print_info() {
    tty_printf(
        "\n===========================================================\n"
        "   kaku - v%d.%d.%d\n\n"
        KAKU_ASCII
        "\n===========================================================\n\n"
        , KAKU_VER_MAJOR, KAKU_VER_MINOR, KAKU_VER_PATCH);
    LOG_INFO("everything initiliazed, kernel running \n\n");
}

static void __init(void) {
    dev_console_init();
    __hw_init();
    __kernel_init();
    tty_init();
    __kernel_print_info();
}

void kernel_main(void) {
    __init();

    int entry_running = elf_load("/bin/genkan", 0, 0);

    if (entry_running < 0) {
        panic("Entry process could not be started!");
    }
    while(1);
}
