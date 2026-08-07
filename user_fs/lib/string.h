#pragma once

#include <stdint.h>
#include <lib/mem.h>

#include "core.h"

SINGLE_H_LIB_FUNC
uint32_t strlen (const char *str) {
    uint32_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

/* both return a heap-allocated, NUL-terminated string (caller must free) */
SINGLE_H_LIB_FUNC
char *itoa(int n) {
    char *buf = malloc(12);           /* sign + 10 digits + NUL */
    if (!buf) return 0;

    uint32_t num;
    int neg = 0;
    if (n < 0) {
        neg = 1;
        num = 0u - (uint32_t)n;
    } else {
        num = (uint32_t)n;
    }

    int i = 11;
    buf[i] = '\0';
    if (num == 0) {
        buf[--i] = '0';
    }
    while (num) {
        buf[--i] = '0' + (num % 10);
        num /= 10;
    }
    if (neg) buf[--i] = '-';

    int j = 0;
    while (buf[i]) buf[j++] = buf[i++];
    buf[j] = '\0';
    return buf;
}

SINGLE_H_LIB_FUNC
char *itoa_hex(uint32_t n) {
    char *buf = malloc(9);            /* 8 hex digits + NUL */
    if (!buf) return 0;

    const char *hex = "0123456789ABCDEF";
    int i = 8;
    buf[i] = '\0';
    do {
        buf[--i] = hex[n & 0xF];
        n >>= 4;
    } while (n);

    int j = 0;
    while (buf[i]) buf[j++] = buf[i++];
    buf[j] = '\0';
    return buf;
}

SINGLE_H_LIB_FUNC
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) { s1++; s2++; }
    return *s1 - *s2;
}

SINGLE_H_LIB_FUNC
char *strcat(char *dst, const char *src) {
    char *d = dst;
    while (*d) d++;
    while ((*d++ = *src++)) ;
    return dst;
}

SINGLE_H_LIB_FUNC
void *memcpy(void *dst, const void *src, uint32_t n) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dst;
}

SINGLE_H_LIB_FUNC
void memset(void *dst, uint8_t val, uint32_t n) {
    uint8_t *d = (uint8_t*)dst;
    while (n--) *d++ = val;
}

SINGLE_H_LIB_FUNC
char *strdup(const char *str) {
    if(str == 0) {
        return 0;
    }
    uint32_t len = strlen(str) + 1;
    char *buf = malloc(len);
    if (buf) {
        for (uint32_t i = 0; i < len; ++i) buf[i] = str[i];
    }
    return buf;
}

static char *__strtok_save;

SINGLE_H_LIB_LOCAL
int __is_delim(char c, const char *delim) {
    while (*delim) {
        if (*delim == c) return 1;
        delim++;
    }
    return 0;
}

SINGLE_H_LIB_FUNC
char *strtok(char *str, const char *delim) {
    if (str == 0) {
        str = __strtok_save;
    }
    if (str == 0) {
        return 0;
    }

    /* skip leading delimiters */
    while (*str && __is_delim(*str, delim)) str++;

    if (*str == '\0') {
        __strtok_save = 0;
        return 0;
    }

    char *token = str;
    while (*str && !__is_delim(*str, delim)) str++;

    if (*str == '\0') {
        __strtok_save = 0;
    } else {
        *str = '\0';
        __strtok_save = str + 1;
    }
    return token;
}

SINGLE_H_LIB_FUNC
int strsplit(const char *s, char **parts, char *sep) {
    char *copy = strdup(s);
    if (!copy) return -1;

    int i = 0;
    char *p = strtok(copy, sep);
    while (p) {
        parts[i] = strdup(p);
        if (!parts[i]) {
            free(copy);
            return -1;
        }
        i++;
        p = strtok(0, sep);
    }

    free(copy);
    return i;
}

SINGLE_H_LIB_FUNC
void strcpy(char *dst, const char *src) {
    while((*dst++ = *src++));
}
