# kaku - kernel - v1.0.0

Basic bootloader & kernel (x86 32bit)

<img src="./assets/kaku_name.png" alt="kaku_logo" width=250/>

***

This is the first major version.
The whole kernel architecture was changed to a simili-modular.
This aims to reduce coupling between modules, using a surface API (see all `mod.h` files).

The userspace also now has some utilities when started with a very basic shell, with builtin `cd` command,
and other commands as ELF binaries (such as `ls` and `cat`).

**Do not expect this to be usable at all.**

See [Makefile](Makefile) to build the image and run it with QEMU.

You will need few things to run it:
- `i686-elf-gcc` to compile the kernel
- `nasm` to assemble the bootloader
- `i686-elf-ld` for the linker
- QEMU to run everything
