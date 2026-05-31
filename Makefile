ASM = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

CFLAGS = -ffreestanding -nostdlib -mgeneral-regs-only -I/usr/lib/gcc/i686-elf/15.2.0/include -Ikernel
LDFLAGS = -T kernel/kernel.ld --oformat binary -Map kernel/kernel.map

KERNEL_SRCS = $(shell find kernel -mindepth 2 -name '*.c')
KERNEL_BIN_DEPS = $(KERNEL_SRCS:.c=.o) kernel/hw/pit/pit_asm.o

#for calculating automatically value for AL in bootloader/boot.asm for loading all sectors for the kernel
KERNEL_SECTORS=$(shell expr $$(wc -c < kernel/kernel.bin) / 512 + 2)

BUILDS=build

all: $(BUILDS)/disk.img

#bootloader targets
bootloader/bootloader.bin: bootloader/boot.asm kernel/kernel.bin
	$(ASM) -f bin -dKERNEL_SECTORS=$(KERNEL_SECTORS) $< -o $@

#kernel related targets
kernel/kernel_entry.o: kernel/kernel_entry.asm
	$(ASM) -f elf32 $< -o $@

kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o $(KERNEL_BIN_DEPS)
	$(LD) $(LDFLAGS) $^ -o $@

#pit specific ASM file
kernel/hw/pit/pit_asm.o: kernel/hw/pit/pit_asm.asm
	$(ASM) -f elf32 $< -o $@

#kernel internal & utils targets

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDS)/disk.img: kernel/kernel.bin bootloader/bootloader.bin
	dd if=/dev/zero of=$@ bs=512 count=131072
	dd if=bootloader/bootloader.bin of=$@ conv=notrunc
	dd if=kernel/kernel.bin of=$@ bs=512 seek=1 conv=notrunc
	echo '2048,,' | sfdisk --label dos $(BUILDS)/disk.img
	mkfs.ext2 -E offset=$$((2048 * 512)) $(BUILDS)/disk.img

fill-disk:
	sudo mount -o loop,offset=$$((2048*512)) build/disk.img /mnt
	printf "noption=1\ngreet=1\n\0" | sudo tee /mnt/config
	sudo touch /mnt/toto.txt
	printf "baba\nbibi\0" | sudo tee /mnt/toto.txt
	sudo mkdir -p /mnt/etc/grub
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
	rm -f bootloader/bootloader.bin kernel/kernel_entry.o kernel/kernel.o kernel/kernel.bin $(KERNEL_BIN_DEPS) $(BUILDS)/disk.img

run: $(BUILDS)/disk.img fill-disk
	qemu-system-i386 -drive format=raw,file=$< -display sdl -serial stdio

run-debug-int: $(BUILDS)/disk.img
	qemu-system-i386 -drive format=raw,file=$< -display sdl -serial stdio -d int
