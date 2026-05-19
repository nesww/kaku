#ifndef PROC_H
#define PROC_H

#include "paging/paging.h"

#define PROC_USER_STACK_TOP_VADDR 0x8000000
#define PROC_USER_FLAGS 0x7 // present | writable | user

typedef enum {
    RUNNING, READY, BLOCKED, ZOMBIE
} proc_state;

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    //
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed)) proc_registers_state;

typedef struct {
    uint32_t proc_id;
    proc_registers_state reg_states;
    uint32_t user_stack;
    uint32_t kernel_stack;
    page_directory *proc_pd;
    proc_state proc_state;
} proc;

proc *proc_create(void(*entry)() /*uint32_t priority*/);
void proc_destroy(proc *p);

#endif
