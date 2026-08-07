#pragma once

#include <lib/stdio.h>

#define TODO(str)                           \
    do {                                    \
        if (str)                            \
            print("TODO: " #str " not yet implemented\n"); \
    } while(0)
