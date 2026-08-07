#pragma once

#define SINGLE_H_LIB_FUNC __attribute__((always_inline)) \
    static inline

//for variadic function that cannot be inlined
#define SINGLE_H_LIB_VARIADIC_FUNC static

/* private/implementation-detail helper: internal linkage only (never
 * exposed in the symbol table), inline so an unused helper in a given TU
 * does not trigger a -Wunused-function warning. */
#define SINGLE_H_LIB_LOCAL static inline
