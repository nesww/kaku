ASM = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

OBJ = obj
BUILDS = build
ISO_DIR = iso
USER_FS = user_fs

USER_HEADERS = $(wildcard $(USER_FS)/lib/*.h) $(USER_FS)/sys/syscalls.h

CFLAGS = -ffreestanding -nostdlib -mgeneral-regs-only -I/usr/lib/gcc/i686-elf/15.2.0/include -Ikernel
LDFLAGS = -T kernel/kernel.ld --oformat binary -Map kernel/kernel.map

KERNEL_SRCS_C = $(shell find kernel -name '*.c')
KERNEL_SRCS_ASM = $(shell find kernel -name '*.asm')
KERNEL_OBJS_C = $(patsubst kernel/%.c,$(OBJ)/kernel/%.o,$(KERNEL_SRCS_C))
KERNEL_OBJS_ASM = $(patsubst kernel/%.asm,$(OBJ)/kernel/%.o,$(KERNEL_SRCS_ASM))
KERNEL_OBJS = $(KERNEL_OBJS_C) $(KERNEL_OBJS_ASM)

KERNEL_SECTORS = $(shell expr $$(wc -c < kernel/kernel.bin) / 512 + 2)

all: $(BUILDS)/disk.img fill-disk

bootloader/bootloader.bin: bootloader/boot.asm kernel/kernel.bin
	$(ASM) -f bin -dKERNEL_SECTORS=$(KERNEL_SECTORS) $< -o $@

kernel/kernel.bin: $(OBJ)/kernel/kernel_entry.o $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(OBJ)/kernel/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/kernel/%.o: kernel/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

$(BUILDS)/disk.img: kernel/kernel.bin bootloader/bootloader.bin
	dd if=/dev/zero of=$@ bs=512 count=131072
	dd if=bootloader/bootloader.bin of=$@ conv=notrunc
	dd if=kernel/kernel.bin of=$@ bs=512 seek=1 conv=notrunc
	echo '2048,,' | sfdisk --label dos $(BUILDS)/disk.img
	mkfs.ext2 -E offset=$$((2048 * 512)) $(BUILDS)/disk.img

$(USER_FS)/obj/_start.o: $(USER_FS)/src/_start.c $(USER_HEADERS)
	$(CC) -ffreestanding -nostdlib -Iuser_fs -c -o $@ $<

# centralized user_fs binary build.
# $(call user_bin,bin,src,obj[,deps])
#   bin  : binary name (-> user_fs/bin/<bin>)
#   src  : source file path
#   obj  : object file path
#   deps : optional extra prerequisites (e.g. sibling binaries)
define user_bin
$(USER_FS)/bin/$(1): $(2) $(USER_FS)/obj/_start.o $(USER_HEADERS) $(4)
	$(CC) -ffreestanding -nostdlib -Iuser_fs -c -o $(3) $(2)
	$(LD) -T $(USER_FS)/cfg/ld/common.ld -o $(USER_FS)/bin/$(1) $(USER_FS)/obj/_start.o $(3)
	rm -rf $(USER_FS)/src/*.o $(USER_FS)/src/utils/*.o
endef

$(eval $(call user_bin,cat,$(USER_FS)/src/utils/cat.c,$(USER_FS)/src/utils/cat.o))
$(eval $(call user_bin,ls,$(USER_FS)/src/utils/ls.c,$(USER_FS)/src/utils/ls.o))
$(eval $(call user_bin,genkan,$(USER_FS)/src/genkan.c,$(USER_FS)/src/genkan.o))
$(eval $(call user_bin,kai,$(USER_FS)/src/kai.c,$(USER_FS)/src/kai.o,$(USER_FS)/bin/ls $(USER_FS)/bin/cat))

fill-disk: $(BUILDS)/disk.img $(USER_FS)/bin/genkan $(USER_FS)/bin/kai
	sudo mount -o loop,offset=$$((2048*512)) build/disk.img /mnt
	sudo cp -r user_fs/* /mnt/
	sudo umount /mnt
	sync
	LOOP=$$(sudo losetup -fP --show build/disk.img) && \
		sudo e2fsck -y $${LOOP}p1 ; \
		sudo losetup -d $$LOOP

check_fs: $(BUILDS)/disk.img
	LOOP=$$(sudo losetup -fP --show build/disk.img) && sudo e2fsck -n $${LOOP}p1 ; sudo losetup -d $$LOOP

mount_fs: $(BUILDS)/disk.img
	sudo mount -o loop,offset=$$((2048*512)) build/disk.img /mnt

clean:
	rm -f bootloader/bootloader.bin kernel/kernel.bin $(BUILDS)/disk.img
	rm -rf $(OBJ)/kernel

run: $(BUILDS)/disk.img fill-disk
	qemu-system-i386 -drive format=raw,file=$< -display gtk,zoom-to-fit=off -serial stdio

run-debug-int: $(BUILDS)/disk.img
	qemu-system-i386 -drive format=raw,file=$< -display sdl -serial stdio -d int
