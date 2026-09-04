NASM ?= nasm

ifeq ($(shell command -v fenwick-gcc 2>/dev/null),)
    CC ?= gcc
else
    CC ?= fenwick-gcc
endif

ifeq ($(shell command -v fenwick-ld 2>/dev/null),)
    LD ?= ld
else
    LD ?= fenwick-ld
endif

ifeq ($(shell command -v fenwick-cargo 2>/dev/null),)
    CARGO ?= cargo
else
    CARGO ?= fenwick-cargo
endif

OBJCOPY ?= objcopy
GRUB ?= grub-mkrescue

ISO = build/FenwickOS.iso

BUILD = build
BOOT_OBJ = $(BUILD)/bootloader.o
KERNEL_OBJ = $(BUILD)/kernel.o
GDT_OBJ = $(BUILD)/gdt.o
GDT_ASM_OBJ = $(BUILD)/gdt_asm.o
SERIAL_ASM_OBJ = $(BUILD)/serial_asm.o
LIBK_SHIM_OBJ = $(BUILD)/libk_shim.o
KERNEL_ELF = $(BUILD)/kernel.elf
LIBK_A = $(BUILD)/liblibk.a
ISO_DIR = $(BUILD)/iso
GRUB_DIR = $(ISO_DIR)/boot/grub
GRUB_CFG = $(GRUB_DIR)/grub.cfg

CARGO_MANIFEST = lib/libk/Cargo.toml
CARGO_TARGET = x86_64-unknown-none

CFLAGS = -ffreestanding -m64 -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -Ilib/libk -Ikernel/gdt

export PATH := toolchain/fenwick-toolchain/bin:$(PATH)

.PHONY: all clean run

all: $(ISO)

run: $(ISO)
	qemu-system-x86_64 -cdrom build/FenwickOS.iso -vga std -serial file:build/qemu-serial.log -display gtk -no-shutdown -d int -D build/qemu.log

$(BUILD):
	mkdir -p $(BUILD)

$(GRUB_DIR):
	mkdir -p $(GRUB_DIR)

$(LIBK_A): $(CARGO_MANIFEST) $(wildcard lib/libk/src/*.rs) lib/libk/src/serial.asm
	$(CARGO) build --manifest-path $(CARGO_MANIFEST) --target $(CARGO_TARGET) --release
	cp lib/libk/target/x86_64-unknown-none/release/liblibk.a $@

$(BOOT_OBJ): boot/arch/x86_64/bootloader.asm | $(BUILD)
	$(NASM) -f elf64 $< -o $@

$(GDT_OBJ): kernel/gdt/gdt.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(GDT_ASM_OBJ): kernel/gdt/gdt.asm | $(BUILD)
	$(NASM) -f elf64 $< -o $@

$(SERIAL_ASM_OBJ): lib/libk/src/serial.asm | $(BUILD)
	$(NASM) -f elf64 $< -o $@

$(LIBK_SHIM_OBJ): lib/libk/src/libk_shim.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_OBJ): kernel/kernel.c | $(BUILD) $(LIBK_A) $(GDT_OBJ) $(GDT_ASM_OBJ) $(SERIAL_ASM_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(BOOT_OBJ) $(KERNEL_OBJ) $(GDT_OBJ) $(GDT_ASM_OBJ) $(LIBK_SHIM_OBJ) $(SERIAL_ASM_OBJ) $(LIBK_A)
	$(LD) -nostdlib -T linker.ld -o $@ $(BOOT_OBJ) $(KERNEL_OBJ) $(GDT_OBJ) $(GDT_ASM_OBJ) $(LIBK_SHIM_OBJ) $(SERIAL_ASM_OBJ) $(LIBK_A)

$(GRUB_CFG): | $(GRUB_DIR)
	echo 'set timeout=0' > $@
	echo 'set default=0' >> $@
	echo 'terminal_input console' >> $@
	echo 'terminal_output console' >> $@
	echo 'menuentry "FenwickOS" {' >> $@
	echo '	multiboot /boot/kernel.elf' >> $@
	echo '}' >> $@

$(ISO): $(KERNEL_ELF) $(GRUB_CFG)
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	$(GRUB) -o $@ $(ISO_DIR)

clean:
	rm -rf $(BUILD) lib/libk/target
