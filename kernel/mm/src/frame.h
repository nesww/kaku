#pragma once

#include <stdint.h>

void fa_init(void);

void *fa_alloc(void);
void fa_free(uint32_t addr);
