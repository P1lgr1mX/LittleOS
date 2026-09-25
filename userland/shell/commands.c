#include "commands.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

int cmd_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("\nAetherOS Shell - Built-in Commands:\n");
    printf("-----------------------------------------------------------------\n");
    for (int i = 0; i < num_builtin_commands; i++) {
        printf("  %s\t\t- %s\n", builtin_commands[i].name, builtin_commands[i].description);
    }
    printf("-----------------------------------------------------------------\n");
    return 0;
}

int cmd_clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* Gọi System Call SYS_CLEAR (5) để Kernel xóa màn hình */
    sys_clear();
    return 0;
}

int cmd_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) {
            printf(" ");
        }
    }
    printf("\n");
    return 0;
}

int cmd_about(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("\n=================================================================\n");
    printf("       AetherOS (LittleOS) - x86 32-bit Operating System         \n");
    printf("       Kernel Mode: Higher-Half Paging (0xC0100000) Ring 0       \n");
    printf("       User Mode  : Flat Binary at 0x00000000 Ring 3             \n");
    printf("       Developed with Pair Programming by AmiyaSokiya & AI       \n");
    printf("=================================================================\n\n");
    return 0;
}

/* Bảng lệnh Shell (Command Table) */
const struct builtin_cmd builtin_commands[] = {
    {"help",  "Hien thi danh sach cac cau lenh ho tro", cmd_help},
    {"clear", "Xoa sach man hinh console",             cmd_clear},
    {"echo",  "In dong van ban ra man hinh",           cmd_echo},
    {"about", "Thong tin he dieu hanh AetherOS",       cmd_about}
};

const int num_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);
