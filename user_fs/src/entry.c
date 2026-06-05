#include "../sys/syscalls.h"
#include <stdint.h>

void _start(void) {
    WRITE("entry: hello from entry!\n");
    WRITE("entry: will keep running\n");
    uint32_t i = 0;
    while(1) {
        if (i == 0) {
            WRITE("entry: still running!\n");
        }
        i++;
        if (i >= 200000000) i = 0;
    }
}
