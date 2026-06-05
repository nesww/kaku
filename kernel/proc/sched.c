#include <hw/pic/pic.h>
#include <panic/panic.h>
#include <lib/dynarray.h>
#include <proc/proc.h>
#include <proc/tss.h>

#include "sched.h"

dynarray *procs = {0};
uint32_t procs_head = 0;
proc *current_proc = 0;
uint8_t sched_initialzed = FALSE;

void scheduler_init(void) {
    procs = dynarray_new(SCHED_PROCS_POOL_INITIAL_SIZE);
    sched_initialzed = TRUE;
}

uint32_t scheduler(uint32_t *regs) {
    if (!sched_initialzed) kernel_panic("SCHEDULER_NOT_INITIALIZED: scheduler was called when it was not initialized!");
    if (procs->count == 0) { pic_sendEOI(0); return (uint32_t)regs; }

    if (current_proc != 0 && current_proc->proc_state == ZOMBIE) {
        uint32_t idx = procs_head;
        dynarray_remove(procs, idx);
        proc_destroy(current_proc);
        current_proc = 0;
        if (procs->count == 0) {
            pic_sendEOI(0);
            return (uint32_t)regs;
        }
        procs_head = procs_head % procs->count;
    } else if (current_proc != 0) {
        current_proc->kernel_stack = (uint32_t)regs;
    }

    procs_head = (procs_head + 1) % procs->count;
    proc *next = dynarray_get(procs, procs_head);
    current_proc = next;
    tss_set_kernel_stack(next->kernel_stack + sizeof(proc_registers_state));
    paging_switch(next->proc_pd);
    pic_sendEOI(0);
    return next->kernel_stack;
}

void scheduler_add_proc(proc *p) {
    if (!sched_initialzed) kernel_panic("SCHEDULER_NOT_INITIALIZED: tried to add a process to the schedular when it was not initialized!");
    if (!p) return;

    dynarray_add(procs, p);
}

proc *scheduler_get_current_proc(void) {
    return current_proc;
}

uint32_t scheduler_on_segfault(uint32_t *regs) {
    if (procs->count == 0) return (uint32_t)regs;
    dynarray_remove(procs, procs_head);
    proc_destroy(current_proc);
    current_proc = 0;
    if (procs->count == 0) {
        kernel_panic("all userspace process died, should not happen - halting");
    };
    procs_head = procs_head % procs->count;
    procs_head = (procs_head + 1) % procs->count;
    proc *next = dynarray_get(procs, procs_head);
    current_proc = next;
    tss_set_kernel_stack(next->kernel_stack + sizeof(proc_registers_state));
    paging_switch(next->proc_pd);
    return next->kernel_stack;
}
