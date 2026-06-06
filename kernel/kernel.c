/*hardware related includes */
#include <fs/fd.h>
#include <io/stdin.h>
#include <panic/panic.h>
#include <hw/idt/idt.h>
#include <hw/pic/pic.h>
#include <hw/pit/pit.h>
#include <hw/serial/serial.h>
#include <hw/vesa/vesa.h>

/*kernel related includes */
#include <alloc/alloc.h>
#include <frame/frame.h>
#include <paging/paging.h>
#include <proc/sched.h>
#include <log/log.h>
#include <fs/ext2/ext2.h>
#include <vfs/vfs.h>
#include <vfs/ext2_vfs.h>
#include <proc/proc.h>
#include <proc/tss.h>
#include <tty/tty.h>

/* libs includes */
// #include <stdint.h>

#define KAKU_VER_MAJOR 0
#define KAKU_VER_MINOR 1
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
    pic_init();
    pit_init();
    idt_init();
}

static void __fs_init(void) {
    fs_ext2_read_superblock();
    fs_ext2_read_bgdt();
    vfs_register_driver(vfs_get_ext2_driver());
}

static void __kernel_init(void) {
    kheap_init();
    fa_init();
    paging_kernel_init();
    vesa_init();
    __fs_init();
    tss_init();
    tss_install();
    scheduler_init();
    stdin_init();
    fd_init_table();
    proc_create_k_idle();
    INTERRUPTS_ENABLE();
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

uint8_t __load_proc(const char *path, uint32_t addr) {
    vfs_node *file = vfs_open(path);
    if (!file) {
        LOG_WARN("given path '%s': file not found\n", path);
        return FALSE;
    }
    proc *proc = proc_create((void(*)())addr);
    uint32_t frame = (uint32_t)fa_alloc();
    paging_map(proc->proc_pd, frame, addr, PROC_USER_FLAGS);
    vfs_read(file, (uint8_t*)PHYS_TO_VIRT(frame), file->size);
    vfs_close(file);
    scheduler_add_proc(proc);
    LOG_INFO("loaded proc from %s at %x, pid=%d\n", path, addr, proc->proc_id);
    return TRUE;
}

static void __init(void) {
    serial_init();
    __hw_init();
    __kernel_init();
    tty_init();
    __kernel_print_info();
}

void kernel_main(void) {
    __init();

    uint32_t entry_address = 0x400000;

    INTERRUPTS_DISABLE();
    uint32_t count = 0;
    uint8_t entry_running =__load_proc("/bin/entry.bin", entry_address);
    INTERRUPTS_ENABLE();

    if (!entry_running) {
        kernel_panic("Entry process could not be started!");
    }
    while(1);
}
