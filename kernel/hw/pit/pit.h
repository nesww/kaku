#pragma once

#define PIT_BASE_FREQ   1193182 //in hertz
#define PIT_TARGET_FREQ 100
#define PIT_DIVISOR     (PIT_BASE_FREQ/PIT_TARGET_FREQ)

#define PIT_CMD_PORT  0x43
#define PIT_DATA_PORT 0x40

void pit_init();
