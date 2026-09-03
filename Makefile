CC = fenwick-gcc
AS = fenwick-nasm
LD = fenwick-ldd
OBJCOPY = fenwind-objcopy
GRUB = grub-mkrescue
CARGO = fenwick-cargo

ISO = build/FenwickOS.iso

BUILD = build
BOOT_OBJ = $(BUILD)/bootloader.o
KERNEL_OBJ = $(BUILD)/kernel.o
LIBK_SHIM_OBJ = $(BUILD)/libk_shim.o
KERNEL_ELF = $(BUILD)/kernel.elf
LIBK_A = $(BUILD)/liblibk.a
ISO_DIR = $(BUILD)/iso
GRUB_DIR = $(ISO_DIR)/boot/grub
GRUB_CFG = $(GRUB_DIR)/grub.cfg

CARGO_MANIFEST = lib/libk/Cargo.toml
CARGO_TARGET = x86_64-unknown-none

export PATH := toolchain/fenwick-toolchain/bin:$(PATH)

.PHONY: all clean

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(GRUB_DIR):
	mkdir -p $(GRUB_DIR)

$(LIBK_A): $(CARGO_MANIFEST) $(wildcard lib/libk/src/*.rs)
	$(CARGO) build --manifest-path $(CARGO_MANIFEST) --target $(CARGO_TARGET) --release
	cp lib/libk/target/x86_64-unknown-none/release/libk.a $@

$(BOOT_OBJ): boot/arch/x86_64/bootloader.asm | $(BUILD)
	$(AS) -f elf64 $< -o $@

$(LIBK_SHIM_OBJ): lib/libk/src/libk_shim.c | $(BUILD)
	$(CC) -ffreestanding -m64 -Ilib/libk -c $< -o $@

$(KERNEL_OBJ): kernel/kernel.c | $(BUILD) $(LIBK_A)
	$(CC) -ffreestanding -m64 -Ilib/libk -c $< -o $@

$(KERNEL_ELF): $(BOOT_OBJ) $(KERNEL_OBJ) $(LIBK_SHIM_OBJ) $(LIBK_A)
	$(LD) -nostdlib -T linker.ld -o $@ $(BOOT_OBJ) $(KERNEL_OBJ) $(LIBK_SHIM_OBJ) $(LIBK_A)

$(GRUB_CFG): | $(GRUB_DIR)
	echo 'set timeout=0' > $@
	echo 'set default=0' >> $@
	echo 'menuentry "FenwickOS" {' >> $@
	echo '	multiboot /boot/kernel.elf' >> $@
	echo '}' >> $@

$(ISO): $(KERNEL_ELF) $(GRUB_CFG)
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	$(GRUB) -o $@ $(ISO_DIR)

clean:
	$(CARGO) clean --manifest-path $(CARGO_MANIFEST)
	rm -rf $(BUILD)
