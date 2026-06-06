#pragma once

#include "../sys/syscalls.h"
#include <stdint.h>


__attribute__((always_inline))
static inline void print(const char *str) {
    SYS_WRITE(str);
}

__attribute__((always_inline))
static inline void println(const char *str) {
    SYS_WRITE(str);
    SYS_WRITE("\n");
}

__attribute__((always_inline))
static inline void read(char *buf, uint32_t len) {
    SYS_READ(buf, len);
}

__attribute__((always_inline))
static inline int open(const char *path, uint32_t flags) {
    uint32_t fd;
    SYS_OPEN(path, flags, fd);
    return (int)fd;
}

__attribute__((always_inline))
static inline void close(int fd) {
    SYS_CLOSE(fd);
}

__attribute__((always_inline))
static inline uint32_t readf(int fd, uint8_t *buf, uint32_t buf_len) {
    uint32_t read_bytes;
    SYS_READ_FILE(fd, buf, buf_len, read_bytes);
    return read_bytes;
}

__attribute__((always_inline))
static inline uint32_t writef(int fd, uint8_t *buf, uint32_t buf_len) {
    uint32_t written_bytes;
    SYS_WRITE_FILE(fd, buf, buf_len, written_bytes);
    return written_bytes;
}
