# Trình biên dịch và công cụ
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-i386

# Cờ biên dịch và liên kết cho Kernel
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
         -I. -Iinclude -Iinclude/arch/x86 -Iinclude/drivers -Iinclude/kernel -Iarch/x86/mmu
ASFLAGS = -f elf
LDFLAGS = -T link.ld -melf_i386 --no-warn-rwx-segments

# Cờ biên dịch và liên kết cho Userland
USER_CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
              -fno-asynchronous-unwind-tables -fno-unwind-tables \
              -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
              -Iuserland/libc/include -Iuserland/shell
USER_LDFLAGS = -T userland/user.ld -melf_i386 --no-warn-rwx-segments

# Thư mục build
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso

# Danh sách mã nguồn Assembly Kernel
ASM_SOURCES = arch/x86/boot/loader.s \
              arch/x86/cpu/io.s \
              arch/x86/cpu/idt.s \
              arch/x86/cpu/isr.s \
              kernel/syscall.s

# Danh sách mã nguồn C Kernel
C_SOURCES = kernel/kmain.c \
            kernel/kheap.c \
            kernel/syscall.c \
            arch/x86/cpu/gdt.c \
            arch/x86/cpu/idt.c \
            arch/x86/cpu/isr.c \
            arch/x86/mmu/paging.c \
            drivers/framebuffer.c \
            drivers/serial.c \
            drivers/pic.c \
            drivers/keyboard.c

# Mã nguồn Userland (Shell & Libc)
USER_ASM_SOURCES = userland/libc/src/entry.s \
                   userland/libc/src/syscall.s

USER_C_SOURCES = userland/libc/src/string.c \
                 userland/libc/src/stdio.c \
                 userland/shell/commands.c \
                 userland/shell/shell.c

# Danh sách file đối tượng Kernel
ASM_OBJECTS = $(patsubst %.s, $(BUILD_DIR)/%.s.o, $(ASM_SOURCES))
C_OBJECTS   = $(patsubst %.c, $(BUILD_DIR)/%.c.o, $(C_SOURCES))
OBJECTS     = $(ASM_OBJECTS) $(C_OBJECTS)

# Danh sách file đối tượng Userland
USER_ASM_OBJECTS = $(patsubst %.s, $(BUILD_DIR)/%.s.o, $(USER_ASM_SOURCES))
USER_C_OBJECTS   = $(patsubst %.c, $(BUILD_DIR)/%.c.o, $(USER_C_SOURCES))
USER_OBJECTS     = $(USER_ASM_OBJECTS) $(USER_C_OBJECTS)

# File nhị phân đầu ra
KERNEL = $(BUILD_DIR)/kernel.elf
USER_ELF = $(BUILD_DIR)/userland/shell.elf
MODULE_BIN = modules/program
OS_ISO = $(BUILD_DIR)/os.iso

# Các file cấu hình GRUB nguồn
GRUB_MENU = arch/x86/boot/grub/menu.lst
GRUB_STAGE2 = arch/x86/boot/grub/stage2_eltorito

.PHONY: all run run-kernel clean

all: $(OS_ISO)

# Quy tắc biên dịch file C Userland
$(BUILD_DIR)/userland/%.c.o: userland/%.c
	@mkdir -p $(dir $@)
	$(CC) $(USER_CFLAGS) $< -o $@

# Quy tắc biên dịch file Assembly Userland
$(BUILD_DIR)/userland/%.s.o: userland/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Quy tắc liên kết User Shell ELF và xuất Flat Binary
$(USER_ELF): $(USER_OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) $(USER_LDFLAGS) $(USER_OBJECTS) -o $@

$(MODULE_BIN): $(USER_ELF)
	@mkdir -p modules
	$(OBJCOPY) -O binary $< $@

# Quy tắc biên dịch file C Kernel
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

# Quy tắc biên dịch file Assembly Kernel
$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Liên kết kernel.elf
$(KERNEL): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

# Đóng gói ISO với GRUB El Torito
$(OS_ISO): $(KERNEL) $(GRUB_MENU) $(GRUB_STAGE2) $(MODULE_BIN)
	@mkdir -p $(ISO_DIR)/boot/grub
	@mkdir -p $(ISO_DIR)/modules
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.elf
	cp $(GRUB_MENU) $(ISO_DIR)/boot/grub/menu.lst
	cp $(GRUB_STAGE2) $(ISO_DIR)/boot/grub/stage2_eltorito
	cp $(MODULE_BIN) $(ISO_DIR)/modules/program
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
run-kernel: $(KERNEL) $(MODULE_BIN)
	$(QEMU) -serial stdio -kernel $(KERNEL) -initrd $(MODULE_BIN)

# Dọn dẹp thư mục build
clean:
	rm -rf $(BUILD_DIR) $(MODULE_BIN)