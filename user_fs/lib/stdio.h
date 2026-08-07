#pragma once

#include <sys/syscalls.h>
#include <lib/string.h>
#include <lib/mem.h>
#include <stdint.h>
#include <stdarg.h>

#include "core.h"

SINGLE_H_LIB_FUNC
void print(const char *str) {
    SYS_WRITE(str);
}

SINGLE_H_LIB_FUNC
void println(const char *str) {
    SYS_WRITE(str);
    SYS_WRITE("\n");
}

SINGLE_H_LIB_FUNC
uint32_t vsnprintf(char *buf, uint32_t size, const char *fmt, va_list args) {
    uint32_t i = 0;
    while (*fmt && i < size - 1) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                const char *s = va_arg(args, const char*);
                while (*s && i < size - 1) buf[i++] = *s++;
            } else if (*fmt == 'd') {
                char *s = itoa(va_arg(args, int));
                if (s) {
                    while (*s && i < size - 1) buf[i++] = *s++;
                    free(s);
                }
            } else if (*fmt == 'x') {
                char *s = itoa_hex(va_arg(args, uint32_t));
                if (s) {
                    while (*s && i < size - 1) buf[i++] = *s++;
                    free(s);
                }
            } else if (*fmt == 'c') {
                buf[i++] = (char)va_arg(args, int);
            }
        } else {
            buf[i++] = *fmt;
        }
        fmt++;
    }
    buf[i] = '\0';
    return i;
}

SINGLE_H_LIB_VARIADIC_FUNC
void printf(const char *fmt, ...) {
    uint32_t size = 256;
    for (int attempt = 0; attempt < 4; ++attempt) {
        char *buf = malloc(size);
        if (!buf) return;

        va_list args;
        va_start(args, fmt);
        uint32_t n = vsnprintf(buf, size, fmt, args);
        va_end(args);

        if (n < size - 1) {
            SYS_WRITE(buf);
            free(buf);
            return;
        }
        free(buf);
        size *= 4;
    }
}

SINGLE_H_LIB_VARIADIC_FUNC
uint32_t snprintf(char *buf, uint32_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    uint32_t n = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return n;
}

SINGLE_H_LIB_FUNC
void read(char *buf, uint32_t len) {
    SYS_READ(buf, len);
}

SINGLE_H_LIB_FUNC
int open(const char *path, uint32_t flags) {
    uint32_t fd;
    SYS_OPEN(path, flags, fd);
    return (int)fd;
}

SINGLE_H_LIB_FUNC
void close(int fd) {
    SYS_CLOSE(fd);
}

SINGLE_H_LIB_FUNC
uint32_t readf(int fd, uint8_t *buf, uint32_t buf_len) {
    uint32_t read_bytes;
    SYS_READ_FILE(fd, buf, buf_len, read_bytes);
    return read_bytes;
}

SINGLE_H_LIB_FUNC
uint32_t writef(int fd, uint8_t *buf, uint32_t buf_len) {
    uint32_t written_bytes;
    SYS_WRITE_FILE(fd, buf, buf_len, written_bytes);
    return written_bytes;
}

/* mirror of the kernel's readdir contract record (KEEP IN SYNC with
 * kernel/syscall/src/syscall.c `dir_entry`). */
typedef struct {
    uint32_t vnode_id;
    uint32_t size;
    uint8_t  is_dir;
    char     name[256];
} dir_entry;

SINGLE_H_LIB_LOCAL
int32_t readdir_raw(const char *path, uint8_t* buf, uint32_t buf_len) {
    int32_t read_bytes;
    SYS_READDIR(path, buf, buf_len, read_bytes);
    return read_bytes;
}

/* fills a heap-allocated array of `dir_entry` (one per directory entry).
 * returns the pointer (caller must free) and sets *count_out to the number
 * of entries, or 0/0 on error. grows the array until it is no longer full. */
SINGLE_H_LIB_FUNC
dir_entry *readdir(const char *path, uint32_t *count_out) {
    uint32_t cap = 16;
    dir_entry *entries = malloc(cap * sizeof(dir_entry));
    if (!entries) {
        if (count_out) *count_out = 0;
        return 0;
    }

    for (;;) {
        int32_t n = readdir_raw(path, (uint8_t*)entries, cap * sizeof(dir_entry));
        if (n < 0) {
            free(entries);
            if (count_out) *count_out = 0;
            return 0;
        }
        if ((uint32_t)n < cap) {
            if (count_out) *count_out = (uint32_t)n;
            return entries;
        }
        /* buffer exactly full: it may still hold more entries, grow and retry */
        cap *= 2;
        dir_entry *ne = realloc(entries, cap * sizeof(dir_entry));
        if (!ne) {
            free(entries);
            if (count_out) *count_out = 0;
            return 0;
        }
        entries = ne;
    }
}
