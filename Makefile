OBJECTS = loader.o io.o framebuffer.o kmain.o
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

# Trình giả lập QEMU cho kiến trúc x86 (32-bit)
QEMU = qemu-system-i386

all: os.iso

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf menu.lst stage2_eltorito
	@# 1. Tạo cây thư mục iso/boot/grub nếu chưa có
	mkdir -p iso/boot/grub
	@# 2. Copy các file cấu hình và nhị phân từ thư mục gốc vào trong
	cp kernel.elf iso/boot/kernel.elf
	cp menu.lst iso/boot/grub/menu.lst
	cp stage2_eltorito iso/boot/grub/stage2_eltorito
	@# 3. Đóng gói ISO bằng genisoimage
	genisoimage -R \
	            -b boot/grub/stage2_eltorito \
	            -no-emul-boot \
	            -boot-load-size 4 \
	            -A os \
	            -input-charset utf8 \
	            -quiet \
	            -boot-info-table \
	            -o os.iso \
	            iso

# Chạy ISO qua QEMU
run: os.iso
	$(QEMU) -cdrom os.iso

# Chạy thẳng kernel mà không cần qua GRUB ISO
run-kernel: kernel.elf
	$(QEMU) -kernel kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o kernel.elf os.iso iso