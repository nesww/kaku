#pragma once

#include <stdint.h>

extern void jump_to_userspace(uint32_t entry, uint32_t user_stack);
