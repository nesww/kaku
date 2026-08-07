#pragma once

#include <stdint.h>

/*===========================================================================
 * syscall — system call dispatcher public API
 *
 * single module, no selective includes needed.
 *===========================================================================*/

#include "src/syscall.h"

static inline uint32_t syscall_dispatch(uint32_t *regs)     { return syscall_handler(regs); }
