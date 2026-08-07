#pragma once

#include <stdint.h>

/*===========================================================================
 * proc — process management public API
 *
 * selective includes: define PROC_<SUBMOD>_IPL before including
 * this header to pull in a specific subsystem.
 *
 *   #define PROC_PROC_IPL
 *   #define PROC_SCHED_IPL
 *   #define PROC_TSS_IPL
 *   #include <proc/mod.h>
 *
 * or define PROC_ALL_IPL to include everything.
 *===========================================================================*/

#ifdef PROC_ALL_IPL
#  define PROC_PROC_IPL
#  define PROC_SCHED_IPL
#  define PROC_TSS_IPL
#endif

/*===========================================================================
 * proc/proc — Process creation and management
 *===========================================================================*/
#ifdef PROC_PROC_IPL
#  define MM_PAGING_IPL
#  include <mm/mod.h>
#  define FS_VFS_IPL
#  include <fs/mod.h>
#  include "src/proc.h"

static inline proc *proc_create_proc(void (*entry)(void))   { return proc_create(entry); }
static inline void  proc_destroy_proc(proc *p)              { proc_destroy(p); }
static inline proc *proc_idle(void)                         { return proc_get_idle(); }
#endif

/*===========================================================================
 * proc/sched — Scheduler
 *===========================================================================*/
#ifdef PROC_SCHED_IPL
#  include "src/sched.h"

static inline void     sched_init(void)                     { scheduler_init(); }
static inline void     sched_add(proc *p)                   { scheduler_add_proc(p); }
static inline uint32_t sched_dispatch(uint32_t *regs)       { return scheduler(regs); }
static inline uint32_t sched_yield(uint32_t *regs)          { return scheduler_yield(regs); }
static inline proc    *sched_get_proc(uint32_t pid)         { return scheduler_get_proc(pid); }
static inline proc    *sched_get_current(void)              { return scheduler_get_current_proc(); }
static inline uint32_t sched_on_segfault(uint32_t *regs)    { return scheduler_on_segfault(regs); }
static inline int      sched_reap_child(uint32_t child_pid) { return scheduler_reap_child(child_pid) ;}
#endif

/*===========================================================================
 * proc/tss — Task State Segment
 *===========================================================================*/
#ifdef PROC_TSS_IPL
#  include "src/tss.h"

static inline void tss_setup(void)                          { tss_init(); }
static inline void tss_install_gdt(void)                    { tss_install(); }
static inline void tss_set_kstack(uint32_t stack_ptr)       { tss_set_kernel_stack(stack_ptr); }
#endif
