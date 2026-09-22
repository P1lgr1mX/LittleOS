#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#define MULTIBOOT_BOOTLOADER_MAGIC  0x2BADB002

/* Các bit cờ trong multiboot_info_t */
#define MULTIBOOT_INFO_MEMORY       0x00000001
#define MULTIBOOT_INFO_BOOTDEV      0x00000002
#define MULTIBOOT_INFO_CMDLINE      0x00000004
#define MULTIBOOT_INFO_MODS         0x00000008
#define MULTIBOOT_INFO_MEM_MAP      0x00000040

/* Cấu trúc mô tả một module do bootloader (GRUB) nạp */
typedef struct multiboot_module {
    unsigned int mod_start;
    unsigned int mod_end;
    unsigned int string;
    unsigned int reserved;
} __attribute__((packed)) multiboot_module_t;

/* Cấu trúc multiboot_info do GRUB truyền qua con trỏ ebx */
typedef struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info;
    unsigned int vbe_mode_info;
    unsigned short vbe_mode;
    unsigned short vbe_interface_seg;
    unsigned short vbe_interface_off;
    unsigned short vbe_interface_len;
} __attribute__((packed)) multiboot_info_t;

#endif /* MULTIBOOT_H */
