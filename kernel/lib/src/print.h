#pragma once

#include <stdint.h>
#include <stdarg.h>

static inline void kvprintf_to(void (*putchar_fn)(char), void(*newline_fn)(void), const char *str, va_list args) {
    uint32_t i = 0;
    while(str[i]) {
        char c = str[i];
        if (c == '%') {
            if (str[i+1]) {
                switch(str[i+1]) {
                    case 's':
                        char *s = va_arg(args, char *);
                        while (*s) putchar_fn(*s++);
                        i++;
                        break;
                    case 'd':
                        int n = va_arg(args, int);
                        if (n >= 10) putchar_fn('0' + n / 10);
                        putchar_fn('0' + n % 10);
                        i++;
                        break;
                    case 'x': {
                        uint32_t x = va_arg(args, uint32_t);
                        char hex[] = "0123456789ABCDEF";
                        putchar_fn('0'); putchar_fn('x');
                        if (x == 0) {
                            putchar_fn('0');
                        } else {
                            int started = 0;
                            for (int j = 7; j >= 0; --j) {
                                uint8_t nibble = (x >> (j*4)) & 0xF;
                                if (nibble != 0 || started) {
                                    putchar_fn(hex[nibble]);
                                    started = 1;
                                }
                            }
                        }
                        i++;
                        break;
                    }
                    default: break;
                }
            } else {
                putchar_fn(c);
            }
        } else if (c == '\n') {
            newline_fn();
        } else if (c == '\t') {
            for (uint32_t i = 0; i < 4; ++i) {
                putchar_fn(' ');
            }
        } else {
            putchar_fn(c);
        }
        i++;
    }
}

static struct {
    char *buf;
    uint32_t size;
    uint32_t cursor;
} snprintf_ctx;

static void __snprintf_putchar(char c) {
    if (snprintf_ctx.cursor < snprintf_ctx.size - 1) {
        snprintf_ctx.buf[snprintf_ctx.cursor++] = c;
    }
}

static void __snprintf_newline(void) {
    __snprintf_putchar('\n');
}

static inline void kvsnprintf(char *buf, uint32_t size, const char *fmt, va_list args) {
    snprintf_ctx.buf = buf;
    snprintf_ctx.size = size;
    snprintf_ctx.cursor = 0;
    kvprintf_to(__snprintf_putchar, __snprintf_newline, fmt, args);
    if (snprintf_ctx.cursor < size)
        buf[snprintf_ctx.cursor] = '\0';
    else
        buf[size - 1] = '\0';
}
