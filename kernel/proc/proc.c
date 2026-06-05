#include <lib/stdmem.h>
#include <frame/frame.h>
#include <log/log.h>
#include <panic/panic.h>
#include <proc/sched.h>

#include "proc.h"

static uint32_t pid_counter = 1;

proc *proc_create(void(*entry)() /*uint32_t priority*/) {
    proc *p = kmalloc(sizeof(proc));
    p->proc_id = pid_counter++;
    p->proc_pd = paging_create_pd();
    uint32_t ks_phys = (uint32_t)fa_alloc();
    uint32_t ks_virt = PHYS_TO_VIRT(ks_phys) + 4096;
    proc_registers_state ctx = {0};
    ctx.eip      = (uint32_t)entry;
    ctx.cs       = 0x1B;
    ctx.eflags   = 0x202;
    ctx.user_esp = PROC_USER_STACK_TOP_VADDR;
    ctx.user_ss  = 0x23;
    p->kernel_stack = (uint32_t)kmemcpy((void*)(ks_virt - sizeof(proc_registers_state)), &ctx, sizeof(proc_registers_state));
    for (uint32_t i = 0; i < 4; ++i) {
        uint32_t frame = (uint32_t)fa_alloc();
        uint32_t vaddr = PROC_USER_STACK_TOP_VADDR - (i+1) * 4096;
        paging_map(p->proc_pd, frame, vaddr, PROC_USER_FLAGS);
    }
    p->user_stack = PROC_USER_STACK_TOP_VADDR;
    p->proc_state = READY;
    return p;
}

void proc_destroy(proc *p) {
    fa_free(VIRT_TO_PHYS(p->kernel_stack + sizeof(proc_registers_state) - 4096));
    for (uint32_t i = 0; i < 4; ++i) {
        uint32_t vaddr = PROC_USER_STACK_TOP_VADDR - (i+1) * 4096;
        uint32_t pd_index = vaddr >> 22;
        page_table *pt = (page_table*)PHYS_TO_VIRT(p->proc_pd->entries[pd_index] & 0xFFFFF000);
        uint32_t pt_index = (vaddr >> 12) & 0x3FF;
        fa_free(pt->entries[pt_index] & 0xFFFFF000);
    }
    fa_free(VIRT_TO_PHYS((uint32_t)p->proc_pd));
    kfree(p);
}
