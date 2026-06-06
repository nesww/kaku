#pragma once

#include <stdint.h>

#include "proc.h"

#define SCHED_PROCS_POOL_INITIAL_SIZE 32

void scheduler_init(void);
void scheduler_add_proc(proc *p);
uint32_t scheduler(uint32_t *regs);
uint32_t scheduler_yield(uint32_t *regs);
proc *scheduler_get_current_proc(void);
uint32_t scheduler_on_segfault(uint32_t *regs);
void scheduler_wake_stdin(
    proc *p, char *proc_buf, uint32_t proc_buf_size,
    uint8_t *stdin_buf, uint8_t stdin_buf_len
) ;
