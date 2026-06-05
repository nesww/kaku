#pragma once

#define WRITE(str) asm volatile( \
    "mov $1, %%eax\n"            \
    "mov %0, %%ebx\n"            \
    "int $0x80\n"                \
    :: "r"(str):                 \
    "eax", "ebx");

#define EXIT() asm volatile( \
    "mov $7, %%eax\n"        \
    "int $0x80\n"            \
    "1: jmp 1b\n"            \
    ::: "eax")
