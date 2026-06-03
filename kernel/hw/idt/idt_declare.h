#include "idt.h"

//intel legacy ISRs
ISR(0)
ISR(1);
ISR(2);
ISR(3);
ISR(4);
ISR(5);
ISR(6);
ISR(7);
ISR_ERR(8);
ISR(9);
ISR_ERR(10);
ISR_ERR(11);
ISR_ERR(12);
ISR_ERR(13);
ISR_ERR(14);
ISR(15);
ISR(16);
ISR(17);
ISR(18);
ISR(19);
ISR(20);
ISR(21);
ISR(22);
ISR(23);
ISR(24);
ISR(25);
ISR(26);
ISR(27);
ISR(28);
ISR(29);
ISR(30);
ISR(31);

// PIC ISRs
// ISR(32); => will now use ASM pit_asm, dedicated for the scheduler
extern void isr_timer_stub(void);

ISR(33);
ISR(34);
ISR(35);
ISR(36);
ISR(37);
ISR(38);
ISR(39);
ISR(40);
ISR(41);
ISR(42);
ISR(43);
ISR(44);
ISR(45);
ISR(46);
ISR(47);

// ISR(128);
extern void isr_syscall_stub(void);
