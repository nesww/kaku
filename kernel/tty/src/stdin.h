#pragma once

#define PROC_PROC_IPL
#include <proc/mod.h>

void stdin_init(void);
void stdin_wait(proc* p, char* buf, uint32_t size);
void stdin_put(char c);
void stdin_pop(void);
void stdin_flush(void);
