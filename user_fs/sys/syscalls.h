#pragma once

#define WRITE     1
#define READ      2
#define OPEN      3
#define CLOSE     4
#define READFILE  5
#define WRITEFILE 6
#define EXIT      7

#define SYS_WRITE(str) asm volatile( \
    "int $0x80\n"                \
    :: "a"(WRITE),"b"(str):          \
    );

#define SYS_READ(buf_addr, buf_size) asm volatile( \
    "int $0x80\n"                              \
    :: "a"(READ), "b"(buf_addr), "c"(buf_size)            \
    : "memory");

#define FD_FLAGS_RO 0x1
#define FD_FLAGS_WO 0x2
#define FD_FLAGS_WR 0x4

#define SYS_OPEN(file_path, flags, fd_out) asm volatile( \
    "int $0x80\n"                              \
    : "=a"(fd_out)                             \
    : "a"(OPEN), "b"(file_path), "c"(flags)       \
    :);

#define SYS_CLOSE(fd) asm volatile(            \
    "int $0x80\n"                              \
    :: "a"(CLOSE), "b"(fd)                         \
    :);

#define SYS_READ_FILE(fd, buf, buf_len, read_bytes_out) do { \
    asm volatile(                                     \
        "int $0x80\n"                                 \
        : "=a"(read_bytes_out)                        \
        : "a"(READFILE), "b"(fd), "c"(buf), "d"(buf_len)     \
        : "memory");                                  \
} while(0)

#define SYS_WRITE_FILE(fd, buf, buf_len, written_bytes_out) do { \
    asm volatile(                                  \
        "int $0x80\n"                              \
        : "=a"(written_bytes_out)                  \
        : "a"(WRITEFILE),"b"(fd), "c"(buf), "d"(buf_len)   \
        : "memory");                               \
} while(0)

#define SYS_EXIT() asm volatile( \
    "mov $7, %%eax\n"            \
    "int $0x80\n"                \
    "1: jmp 1b\n"                \
    ::: "eax")
