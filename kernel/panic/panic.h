#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include "hw/serial/serial.h"
#include "tty/tty.h"
#include "types.h"

#define PANIC_HLT() \
    __asm__ volatile("cli; hlt")

static inline void __panic_init(void) {
    tty_set_bg(0xFFb3bcff);
    tty_set_fg(0xBB333333);
    tty_clear();
    tty_printf("kaku ---- kernel panicked!\n\n");
}

static void kernel_panic_isr(int int_num, struct interrupt_frame *int_frame) {
    __panic_init();
    tty_printf("KERNEL PANIC\n> Interrupt: %d\n> State frame: \n>> IP (where): %x\n>> CPU flags: %x\n>> CS (code segment): %x\n>> SS (stack segment): %x\n>> SP (stack pointer): %x\n"
        , int_num, int_frame->ip, int_frame->flags, int_frame->cs, int_frame->ss, int_frame->sp);
    SERIAL_PANIC("KERNEL PANIC\n> Interrupt: %d\n> State frame: \n>> IP (where): %x\n>> CPU flags: %x\n>> CS (code segment): %x\n>> SS (stack segment): %x\n>> SP (stack pointer): %x\n"
        , int_num, int_frame->ip, int_frame->flags, int_frame->cs, int_frame->ss, int_frame->sp);
    PANIC_HLT();
}

static void kernel_panic(const char *msg) {
    __panic_init();
    tty_printf("KERNEL PANIC\n> %s\n", msg);
    SERIAL_PANIC("KERNEL PANIC\n> %s\n", msg);

    PANIC_HLT();
}

static void kernel_panicf(const char *msg, ...) {
    __panic_init();
    va_list args;
    va_start(args, msg);
    tty_vprintf(msg, args);
    PANIC_HLT();
}

#endif //KERNEL_PANIC_H
