#ifndef KPRINT_H
#define KPRINT_H

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
                        for (int j = 7; j >= 0; --j)
                            putchar_fn(hex[(x >> (j*4)) & 0xF]);
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
        } else {
            putchar_fn(c);
        }
        i++;
    }
}

#endif
