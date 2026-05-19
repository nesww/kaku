#include "serial.h"
#include "lib/core.h"
#include "panic/panic.h"
#include "hw/vga/vga.h"
#include "hw/io/io.h"
#include <stdarg.h>
#include <stdint.h>

uint8_t serial_initialized = FALSE;

void serial_init(void) {
    outb(SERIAL_BASE + 1, 0x00); //no interrupts
    outb(SERIAL_BASE + 3, 0x80); //enable DLAB
    outb(SERIAL_BASE + 0, 0x03); //divisor low (38400 baud)
    outb(SERIAL_BASE + 1, 0x00); //divisor high
    outb(SERIAL_BASE + 3, 0x03); //8 bits, no parity, 1 stop bit
    outb(SERIAL_BASE + 2, 0xC7); //enable fifo
    serial_initialized = TRUE;
    serial_printf("serial initialized\n");
}

void serial_putchar(char c) {
    if (!serial_initialized) kernel_panic("SERIAL_NOT_INITIALIZED: tried to putchar to serial port when not initialized!");
    while((inb(SERIAL_COM1_LINE_STATUS_PORT) & 0x20) == 0);
    outb(SERIAL_COM1_DATA_REGISTER_PORT, c);
}

static void __serial_newline(void) {
    serial_putchar('\r');
    serial_putchar('\n');
}

void serial_printf(const char *str, ...) {
    va_list args;
    va_start(args, str);
    vga_vprintf_to(serial_putchar, __serial_newline, str, args);
    va_end(args);
}
