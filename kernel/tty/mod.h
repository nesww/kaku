#pragma once

#include <stdint.h>
#include <stdarg.h>

/*===========================================================================
 * tty — terminal public API
 *
 * selective includes: define TTY_<SUBMOD>_IPL before including
 * this header to pull in a specific subsystem.
 *
 *   #define TTY_TTY_IPL
 *   #define TTY_STDIN_IPL
 *   #include <tty/mod.h>
 *
 * or define TTY_ALL_IPL to include everything.
 *===========================================================================*/

#ifdef TTY_ALL_IPL
#  define TTY_TTY_IPL
#  define TTY_STDIN_IPL
#endif

/*===========================================================================
 * tty/tty — terminal output
 *
 * internal functions already use tty_* naming, exposed directly.
 *===========================================================================*/
#ifdef TTY_TTY_IPL
#  include "src/tty.h"
#endif

/*===========================================================================
 * tty/stdin — Terminal input
 *===========================================================================*/
#ifdef TTY_STDIN_IPL
#  include "src/stdin.h"
#endif
