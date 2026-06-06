#pragma once

#include "../sys/syscalls.h"

#define TODO(str)                           \
    do {                                    \
        if (str)                            \
            SYS_WRITE("TODO: " #str " not yet implemented\n"); \
    } while(0)
