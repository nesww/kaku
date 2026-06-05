#include <hw/io/io.h>

#include "pit.h"

void pit_init(void) {
    outb(PIT_CMD_PORT, 0x36);

    outb(PIT_DATA_PORT, (PIT_DIVISOR & 0xFF));
    outb(PIT_DATA_PORT, (PIT_DIVISOR >> 8));
}
