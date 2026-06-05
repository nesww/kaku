#pragma once

#include <stdint.h>

#include "proc.h"

#define SCHED_PROCS_POOL_INITIAL_SIZE 32

void scheduler_init(void);
void scheduler_add_proc(proc *p);
uint32_t scheduler(uint32_t *regs);
proc *scheduler_get_current_proc(void);
uint32_t scheduler_on_segfault(uint32_t *regs);
