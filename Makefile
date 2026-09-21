# Trình biên dịch và công cụ
CC = gcc
AS = nasm
LD = ld
QEMU = qemu-system-i386

# Cờ biên dịch và liên kết
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
         -I. -Idrivers -Ikernel
ASFLAGS = -f elf
LDFLAGS = -T link.ld -melf_i386

# Thư mục build
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso

# Danh sách mã nguồn
ASM_SOURCES = boot/loader.s \
              drivers/io.s

C_SOURCES = kernel/kmain.c \
            drivers/framebuffer.c \
            drivers/serial.c

# Danh sách file đối tượng (.o) nằm trong build/
OBJECTS = $(patsubst %.s, $(BUILD_DIR)/%.o, $(ASM_SOURCES)) \
          $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))

# File nhị phân đầu ra
KERNEL = $(BUILD_DIR)/kernel.elf
OS_ISO = $(BUILD_DIR)/os.iso

# Các file cấu hình GRUB nguồn
GRUB_MENU = boot/grub/menu.lst
GRUB_STAGE2 = boot/grub/stage2_eltorito

.PHONY: all run run-kernel clean

all: $(OS_ISO)

# Quy tắc biên dịch file C sang file .o
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

# Quy tắc biên dịch file Assembly sang file .o
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Liên kết kernel.elf
$(KERNEL): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

# Đóng gói ISO với GRUB El Torito
$(OS_ISO): $(KERNEL) $(GRUB_MENU) $(GRUB_STAGE2)
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.elf
	cp $(GRUB_MENU) $(ISO_DIR)/boot/grub/menu.lst
	cp $(GRUB_STAGE2) $(ISO_DIR)/boot/grub/stage2_eltorito
	genisoimage -R \
	            -b boot/grub/stage2_eltorito \
	            -no-emul-boot \
	            -boot-load-size 4 \
	            -A os \
	            -input-charset utf8 \
	            -quiet \
	            -boot-info-table \
	            -o $(OS_ISO) \
	            $(ISO_DIR)

# Chạy ISO qua QEMU
run: $(OS_ISO)
	$(QEMU) -serial stdio -cdrom $(OS_ISO)

# Chạy trực tiếp kernel ELF qua QEMU
run-kernel: $(KERNEL)
	$(QEMU) -serial stdio -kernel $(KERNEL)

# Dọn dẹp thư mục build
clean:
	rm -rf $(BUILD_DIR)