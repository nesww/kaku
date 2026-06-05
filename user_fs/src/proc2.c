#include "../sys/syscalls.h"

#include<stdint.h>

__attribute__((noreturn))
void _start(void) {
    const char *msg = "hello from proc2!\n";

    for (int i = 0; i < 10; ++i) {
        WRITE(msg);
    }

    volatile uint32_t *bad_ptr = (uint32_t*)0x1;
    *bad_ptr = 1; // should segfault and get the process killed
    __builtin_unreachable();

    EXIT();
}
