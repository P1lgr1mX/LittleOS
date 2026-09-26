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

    /* Invoke SYS_CLEAR (5) system call to clear the console display */
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

/* Built-in command table */
const struct builtin_cmd builtin_commands[] = {
    {"help",  "Display information about builtin commands", cmd_help},
    {"clear", "Clear the terminal screen",                  cmd_clear},
    {"echo",  "Display a line of text",                     cmd_echo},
    {"about", "Display system and kernel information",       cmd_about}
};

const int num_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);
