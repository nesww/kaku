#pragma once

#include <stdint.h>

#define SYS_WRITE      0x1
#define SYS_READ       0x2
#define SYS_OPEN       0x3
#define SYS_CLOSE      0x4
#define SYS_READ_FILE  0x5
#define SYS_WRITE_FILE 0x6
#define SYS_EXIT       0x7

#define SYS_EXEC 0x8
#define SYS_KILL 0x9

#define SYS_WAITPID 0xa
#define SYS_READDIR 0xb

#define SYS_SBRK   0xc
#define SYS_MMAP   0xd
#define SYS_MUNMAP 0xe

#define SYS_GETPID 0xf

uint32_t syscall_handler(uint32_t *regs);
