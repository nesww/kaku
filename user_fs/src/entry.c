#include "../sys/syscalls.h"
#include "../lib/string.h"
#include "../lib/stdio.h"

//genkan will be the PID1 for my OS, if killed, nukes the kernel
void _start(void) {
    print("\n\n ==== ==== ==== ==== ==== ==== ==== ====\n\n");
    print("genkan: starting up...\n");
    //will do something someday maybe who knows?

    int fd = open("/lib/stdmem.h", FD_FLAGS_RO);
    print("got a fd after open: ");
    println(itoa(fd));

    char buf[2048];
    readf(fd, (uint8_t*)buf, 2048);
    close(fd);

    println("following, the contents read from readf of /lib/stdmem.h:");
    println(buf);

    fd = open("/lib/stdmem.h", FD_FLAGS_WR);
    char buf_2[42] = "I am the new content for /lib/stdmem.h :)";
    writef(fd, (uint8_t*)buf_2, 42);
    println("wrote new content for /lib/stdmem.h");
    readf(fd, (uint8_t*)buf, 42);
    println("new content after write:");
    println(buf);

    close(fd);


    char in_buf[512];

    println("genkan: ready");

    while(1) {
        println("(gkn) Type some text:");
        print("(gkn) > ");
        read(in_buf, 512);
        println("\n(gkn) you typed the following string:");
        println(in_buf);
    }
}
