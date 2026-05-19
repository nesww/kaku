#include "proc.h"
#include "lib/stdmem.h"
#include "frame/frame.h"

static uint32_t pid_counter = 1;

proc *proc_create(void(*entry)() /*uint32_t priority*/) {
    proc *p = kmalloc(sizeof(proc));
    p->proc_id = pid_counter++;
    p->proc_pd = paging_create_pd();

    p->kernel_stack = (uint32_t)fa_alloc() + 4096;
    proc_registers_state ctx = {0};
    ctx.eip = (uint32_t)entry;
    ctx.cs = 0x08;
    ctx.eflags = 0x202;
    p->kernel_stack = (uint32_t)kmemcpy((void*)(p->kernel_stack - sizeof(proc_registers_state)), &ctx, sizeof(proc_registers_state));
    for (uint32_t i = 0; i < 4; ++i) {
        uint32_t frame = (uint32_t)fa_alloc();
        uint32_t vaddr = PROC_USER_STACK_TOP_VADDR - (i+1) * 4096;
        paging_map(p->proc_pd, frame, vaddr, PROC_USER_FLAGS);
    }
    p->user_stack = PROC_USER_STACK_TOP_VADDR;
    p->reg_states.esp = PROC_USER_STACK_TOP_VADDR;
    p->reg_states.eip = (uint32_t)entry;
    p->reg_states.eflags = 0x202;
    p->reg_states.cs = 0x08; //from GDT -> code section

    p->proc_state = READY;
    return p;
}

void proc_destroy(proc *p) {
    fa_free(p->kernel_stack + sizeof(proc_registers_state) - 4096);
    page_table *pt;
    for (uint32_t i = 0; i < 4; ++i) {
        uint32_t vaddr = PROC_USER_STACK_TOP_VADDR - (i+1) * 4096;
        uint32_t pd_index = vaddr >> 22;
        pt = (page_table*)(p->proc_pd->entries[pd_index] & 0xFFFFF000);
        uint32_t pt_index = (vaddr >> 12) & 0x3FF;
        fa_free(pt->entries[pt_index] & 0xFFFFF000);
    }
    fa_free((uint32_t)pt);
    fa_free((uint32_t)p->proc_pd);
    kfree(p);
}
