#pragma once

#include <stdarg.h>
#include <types.h>

/*===========================================================================
 * panic — kernel panic public API
 *
 * single module, no selective includes needed.
 *===========================================================================*/

#define DEV_CONSOLE_IPL
#define TTY_TTY_IPL
#include <dev/mod.h>
#include <tty/mod.h>
#include <log/mod.h>

#include "src/panic.h"

static inline void panic(const char *msg)                    { kernel_panic(msg); }

static inline void panicf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kernel_panicf(fmt, args);
    va_end(args);
}
