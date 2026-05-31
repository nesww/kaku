[BITS 32]
extern kernel_main

mov esp, 0x9ff00

call kernel_main
hlt
