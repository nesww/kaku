#define HW_IO_IPL
#define HW_TIMER_IPL

#include <hw/mod.h>

#include "pit.h"

void pit_init(void) {
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_DATA_PORT, (PIT_DIVISOR & 0xFF));
    outb(PIT_DATA_PORT, (PIT_DIVISOR >> 8));
}

void hw_timer_init(uint32_t freq_hz) {
    uint32_t divisor = HW_PIT_BASE_FREQ / freq_hz;
    outb(HW_PIT_CMD_PORT, 0x36);
    outb(HW_PIT_DATA_PORT, (uint8_t)(divisor & 0xFF));
    outb(HW_PIT_DATA_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}
