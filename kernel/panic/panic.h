#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include "hw/vga/vga.h"
#include "hw/serial/serial.h"
#include "types.h"

#define PANIC_HLT() \
    __asm__ volatile("cli; hlt")

static inline void __panic_init(void) {
    vga_cclear(VGA_BLUE);
    vga_setbackground_color(VGA_BLUE);
}

static void kernel_panic_isr(int int_num, struct interrupt_frame *int_frame) {
    __panic_init();
    vga_printf("KERNEL PANIC\n> Interrupt: %d\n> State frame: \n>> IP (where): %x\n>> CPU flags: %x\n>> CS (code segment): %x\n>> SS (stack segment): %x\n>> SP (stack pointer): %x\n"
        , int_num, int_frame->ip, int_frame->flags, int_frame->cs, int_frame->ss, int_frame->sp);
    SERIAL_PANIC("KERNEL PANIC\n> Interrupt: %d\n> State frame: \n>> IP (where): %x\n>> CPU flags: %x\n>> CS (code segment): %x\n>> SS (stack segment): %x\n>> SP (stack pointer): %x\n"
        , int_num, int_frame->ip, int_frame->flags, int_frame->cs, int_frame->ss, int_frame->sp);
    PANIC_HLT();
}

static void kernel_panic(const char *msg) {
    __panic_init();
    vga_printf("KERNEL PANIC\n> %s\n", msg);
    SERIAL_PANIC("KERNEL PANIC\n> %s\n", msg);

    PANIC_HLT();
}

static void kernel_panicf(const char *msg, ...) {
    __panic_init();
    va_list args;
    va_start(args, msg);
    vga_printf("KERNEL_PANIC\n");
    vga_vprintf(msg, args);
    PANIC_HLT();
}

#endif //KERNEL_PANIC_H
