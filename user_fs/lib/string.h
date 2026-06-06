#pragma once

#include <stdint.h>

static char __itoa_buf[12]; //no malloc so only 0-9 + sign & null terminator

__attribute__((always_inline))
static inline const char *itoa(int n) {
    if (n == 0) { __itoa_buf[0] = '0'; __itoa_buf[1] = '\0'; return __itoa_buf; }
    uint8_t neg = n < 0;
    if (neg) n = -n;
    uint8_t i = 11;
    __itoa_buf[i] = '\0';
    while (n > 0) {
        __itoa_buf[--i] = '0' + (n % 10);
        n /= 10;
    }
    if (neg) __itoa_buf[--i] = '-';
    return __itoa_buf + i;
}
