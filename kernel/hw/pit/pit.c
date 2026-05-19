#include "pit.h"

#include "hw/io/io.h"
#include "hw/serial/serial.h"

void pit_init(void) {
    outb(PIT_CMD_PORT, 0x36);

    outb(PIT_DATA_PORT, (PIT_DIVISOR & 0xFF));
    outb(PIT_DATA_PORT, (PIT_DIVISOR >> 8));
    serial_printf("pit initialized\n");
}
