#include <lib/stdio.h>
#include <lib/stdlib.h>

//genkan will be the PID1 for OS, if killed, nukes the kernel
void main(void) {

    if (getpid() != 1) {
        printf("genkan: error: already running\n");
        return;
    }

    print("\n\n ==== ==== ==== ==== ==== ==== ==== ====\n\n");
    println("*** genkan: starting up...\n");

    int kai_pid = exec("/bin/kai", 0);
    if (kai_pid < 0) {
        println("*** shell could not be started! genkan will idle\n");
    }

    waitpid(kai_pid);
    printf("*** genkan: shell was killed or exited. genkan will now idle\n");

    while(1);
}
