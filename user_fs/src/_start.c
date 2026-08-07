#include <stdint.h>

extern int main(int argc, char **argv);

char **environ;   /* defined here; referenced extern by libs */

/*
 * program entry point. The scheduler lays out, at the top of the user stack:
 *   [argv[0]..argv[argc-1]] [NULL] [envp[0]..envp[envc-1]] [NULL] [strings]
 * and sets the initial esp to point at argv[0].
 *
 * this must be `naked`: a normal C function would emit a prologue
 * (push ebp / sub esp, N) and reading `esp` afterwards would point below the
 * argv array, so argv[0] would read a zero -> argc == 0.
 */
__attribute__((naked, noreturn))
void _start(void) {
    __asm__ volatile(
        "mov %esp, %eax\n"            /* eax = argv base          */
        "xor %ecx, %ecx\n"            /* ecx = argc = 0           */
        "1:\n"
        "cmpl $0, (%eax,%ecx,4)\n"    /* while (argv[argc])       */
        "je 2f\n"
        "incl %ecx\n"
        "jmp 1b\n"
        "2:\n"
        "lea 1(%ecx), %edx\n"         /* edx = argc + 1           */
        "shl $2, %edx\n"              /* * 4                      */
        "add %eax, %edx\n"            /* edx = argv + (argc+1)*4  */
        "mov %edx, environ\n"         /* environ = envp           */
        "push %eax\n"                 /* argv (2nd arg)           */
        "push %ecx\n"                 /* argc (1st arg)           */
        "call main\n"
        "add $8, %esp\n"              /* cdecl caller cleanup     */
        "mov %eax, %ebx\n"            /* exit code -> ebx         */
        "mov $7, %eax\n"              /* SYS_EXIT = 7             */
        "int $0x80\n"
        "hlt\n"
    );
}
