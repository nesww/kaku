#pragma once

#include <proc/proc.h>

void stdin_init(void);
void stdin_wait(proc* p, char* buf, uint32_t size);
void stdin_put(char c);
void stdin_pop(void);
void stdin_flush(void);
