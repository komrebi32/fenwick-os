CC = fenwick-gcc
AS = fenwick-nasm
LD = fenwick-ldd
OBJCOPY = fenwind-objcopy
GRUB = grub-mkrescue
ISO = build/FenwickOS.iso

BUILD = build
BOOT_OBJ = $(BUILD)/bootloader.o
KERNEL_OBJ = $(BUILD)/kernel.o
KERNEL_ELF = $(BUILD)/kernel.elf
ISO_DIR = $(BUILD)/iso
GRUB_DIR = $(ISO_DIR)/boot/grub
GRUB_CFG = $(GRUB_DIR)/grub.cfg

.PHONY: all clean

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(GRUB_DIR):
	mkdir -p $(GRUB_DIR)

$(BOOT_OBJ): boot/arch/x86_64/bootloader.asm | $(BUILD)
	$(AS) -f elf64 $< -o $@

$(KERNEL_OBJ): kernel/kernel.c | $(BUILD)
	$(CC) -ffreestanding -m64 -c $< -o $@

$(KERNEL_ELF): $(BOOT_OBJ) $(KERNEL_OBJ)
	$(LD) -nostdlib -o $@ $^

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
	rm -rf $(BUILD)
