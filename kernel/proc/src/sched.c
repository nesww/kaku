#define HW_IO_IPL
#define HW_IRQ_IPL
#define LIB_ARRAY_IPL
#define LIB_MEM_IPL
#define LIB_STR_IPL
#define MM_FRAME_IPL
#define MM_PAGING_IPL
#define PROC_PROC_IPL
#define PROC_TSS_IPL
#define FS_VFS_IPL
#define FS_FD_IPL

#include <hw/mod.h>
#include <panic/mod.h>
#include <lib/mod.h>
#include <proc/mod.h>

#include "sched.h"

dynarray *procs = {0};
uint32_t procs_head = 0;
proc *current_proc = 0;
uint8_t sched_initialzed = FALSE;

void scheduler_init(void) {
    procs = dynarray_new(SCHED_PROCS_POOL_INITIAL_SIZE);
    sched_initialzed = TRUE;
}

static void __sched_flush_proc_bufs(proc *p) {
    if (p->stdin_kernel_buf) {
        kmemcpy(p->stdin_user_buf, p->stdin_kernel_buf, p->stdin_kernel_buf_len);
        ((char*)p->stdin_user_buf)[p->stdin_kernel_buf_len] = '\0';
        p->stdin_kernel_buf_len = 0;
        p->stdin_kernel_buf = 0;
    }
}

static proc *pick_next_ready_proc(void) {
    uint32_t tried = 0;
    proc *next;
    do {
        procs_head = (procs_head+1) % procs->count;
        next = dynarray_get(procs, procs_head);
        tried++;
    } while ((next->proc_state == BLOCKED || next->proc_state == ZOMBIE) && tried < procs->count);
    return (next->proc_state == BLOCKED || next->proc_state == ZOMBIE) ? 0 : next;
}


static uint32_t __scheduler_core(uint32_t *regs) {
    if (!sched_initialzed) kernel_panic("SCHEDULER_NOT_INITIALIZED: scheduler was called when it was not initialized!");
    if (procs->count == 0) return (uint32_t)regs;
    if (current_proc != 0 && current_proc->proc_state == ZOMBIE) {
        current_proc = 0;
        if (procs->count == 0) {
            return (uint32_t)regs;
        }
        procs_head = procs_head % procs->count;
    } else if (current_proc != 0) {
        current_proc->kernel_stack = (uint32_t)regs;
    }
    uint32_t tried = 0;
    proc *next = pick_next_ready_proc();
    if (!next) {
        proc *idle = proc_get_idle();
        current_proc = 0;
        mm_paging_switch(idle->proc_pd);
        return idle->kernel_stack;
    }
    current_proc = next;
    tss_set_kernel_stack(next->kernel_stack + sizeof(proc_registers_state));
    mm_paging_switch(next->proc_pd);
    __sched_flush_proc_bufs(next);

    if (next->proc_state == STARTING) {
        uint32_t addr = PROC_USER_STACK_TOP_VADDR;
        uint32_t argv_ptrs[next->argc ? next->argc : 1];
        uint32_t env_ptrs[next->envc ? next->envc : 1];

        for (uint32_t i = 0; i < next->argc; ++i) {
            char *arg = next->argv[i];
            uint32_t arg_len = kstrlen(arg);
            addr -= arg_len + 1;
            kmemcpy((uint32_t*)addr, arg, arg_len + 1);
            argv_ptrs[i] = addr;
        }
        for (uint32_t i = 0; i < next->envc; ++i) {
            char *env = next->env[i];
            uint32_t env_len = kstrlen(env);
            addr -= env_len + 1;
            kmemcpy((uint32_t*)addr, env, env_len + 1);
            env_ptrs[i] = addr;
        }

        /* envp array sits ABOVE argv on the stack */
        addr -= sizeof(char*);
        uint32_t z = 0;
        kmemcpy((uint32_t*)addr, &z, sizeof(char*));

        for (int i = (int)next->envc - 1; i >= 0; --i) {
            addr -= sizeof(char*);
            kmemcpy((uint32_t*)addr, &env_ptrs[i], sizeof(char*));
        }

        /* argv array sits at the bottom; argv[0] == user_esp */
        addr -= sizeof(char*);
        kmemcpy((uint32_t*)addr, &z, sizeof(char*));

        for (int i = (int)next->argc - 1; i >= 0; --i) {
            addr -= sizeof(char*);
            kmemcpy((uint32_t*)addr, &argv_ptrs[i], sizeof(char*));
        }

        proc_registers_state *p_kstack = (proc_registers_state*)next->kernel_stack;
        p_kstack->user_esp = addr;
        next->proc_state = READY;
    }
        return next->kernel_stack;
    }

uint32_t scheduler(uint32_t *regs) {
    uint32_t result = __scheduler_core(regs);
    pic_sendEOI(0);
    return result;
}

uint32_t scheduler_yield(uint32_t *regs) {
    return __scheduler_core(regs);
}

void scheduler_add_proc(proc *p) {
    if (!sched_initialzed) kernel_panic("SCHEDULER_NOT_INITIALIZED: tried to add a process to the schedular when it was not initialized!");
    if (!p) return;

    dynarray_add(procs, p);
}

proc *scheduler_get_proc(uint32_t pid) {
    for (uint32_t i = 0; i < procs->count; ++i) {
        proc *p = dynarray_get(procs, i);
        if (p && p->proc_id == pid) return p;
    }
    return 0;
}

proc *scheduler_get_current_proc(void) {
    return current_proc;
}

uint32_t scheduler_on_segfault(uint32_t *regs) {
    if (procs->count == 0) return (uint32_t)regs;
    scheduler_notify_parent(current_proc);
    current_proc = 0;
    if (procs->count == 0) {
        kernel_panic("all userspace process died, should not happen - halting");
    }
    procs_head = procs_head % procs->count;

    proc *next = pick_next_ready_proc();
    if (!next) {
        proc *idle = proc_get_idle();
        current_proc = 0;
        mm_paging_switch(idle->proc_pd);
        return idle->kernel_stack;
    }
    current_proc = next;
    tss_set_kernel_stack(next->kernel_stack + sizeof(proc_registers_state));
    mm_paging_switch(next->proc_pd);
    return next->kernel_stack;
}

void scheduler_wake_stdin(
    proc *p, char *proc_buf, uint32_t proc_buf_size,
    uint8_t *stdin_buf, uint8_t stdin_buf_len
) {
    p->stdin_kernel_buf = stdin_buf;
    p->stdin_kernel_buf_len = stdin_buf_len;
    p->proc_state = READY;
}

void scheduler_notify_parent(proc *child) {
    if (child->parent_pid == 0) return;

    proc *parent = scheduler_get_proc(child->parent_pid);
    if(!parent) return;

    if(parent->proc_state == BLOCKED && parent->waiting_for == child->proc_id) {
        parent->proc_state = READY;
        parent->waiting_for = 0;
    }
}

int scheduler_reap_child(uint32_t child_pid) {
    proc *child = scheduler_get_proc(child_pid);
    if (!child || child->proc_state != ZOMBIE) {
        return -1;
    }

    int exit_code = child->exit_code;

    for (uint32_t i = 0; i < procs->count; ++i) {
        if (dynarray_get(procs, i) == child) {
            dynarray_remove(procs, i);
            break;
        }
    }
    proc_destroy(child);
    return exit_code;
}
